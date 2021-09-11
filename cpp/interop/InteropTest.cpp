#include "InteropTest.h"

#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"
#include "RpcClient.h"
#include "RpcStreamReader.h"

#include <iostream>
#include <string>
#include <random>
#include <future>

static inline auto generateUniqueKey()
{
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<char> uniform_dist('a', 'z');
    std::stringstream ss;

    for(int i = 0; i < 10; i++)
        ss << uniform_dist(e1);

    return ss.str();
}

static inline auto startServiceThread(std::shared_ptr<Rpc> uut)
{
    return std::thread([uut](){
        while(((rpc::FdStreamAdapter*)uut.get())->receive([&uut](auto message)
        {    
            if(auto a = message.access(); auto err = uut->process(a))
                std::cerr << "RPC SRV: " << err << std::endl;

            return true;
        }));
    });
}

static inline void runUnknownMethodLookupTest(std::shared_ptr<Rpc> uut)
{
	std::promise<bool> p;
	auto ret = p.get_future();

	const char* err = uut->Endpoint::lookup(syms::nope, [p{std::move(p)}](auto&, bool done, auto) mutable { p.set_value(done); });
	assert(err == nullptr);

	bool failed = ret.get() == false;

    assert(failed);
}

static inline void runEchoTest(std::shared_ptr<Rpc> uut)
{
	bool done = false;

	auto key1 = generateUniqueKey();
	std::vector<char> keyv;
	std::copy(key1.begin(), key1.end(), std::back_inserter(keyv));
    uut->echo(keyv, [&done, key1](const std::string& result)
	{
    	assert(result == key1);
    	done = true;
	});

	auto key2 = generateUniqueKey();
    std::stringstream ss;
    for(char c: uut->echo<std::list<char>>(key2).get())
    {
    	ss.write(&c, 1);
    }

    auto result = ss.str();
    assert(result == key2);

    assert(done);
}

//static inline void runStreamGeneratorTest(std::shared_ptr<Rpc> uut)
//{
//    auto open = uut->lookupFuture(syms::open).get();
//
//    struct Tester
//    {
//        uint16_t state;
//        const uint16_t mod;
//        syms::Methods methods;
//
//        void runTest(decltype(uut)& uut)
//        {
//            int idx = 0;
//            for(auto v: uut->asyncCall(std::get<1>(methods), 5u).get())
//            {
//                idx++;
//                assert(v == state);
//                state = state * state % mod;
//            }
//            assert(idx == 5);
//        }
//
//        void close(decltype(uut)& uut) {
//            uut->simpleCall(std::get<0>(methods));
//        }
//    };
//
//    Tester tc1{3, 31, uut->asyncCall(open, uint8_t(3), uint8_t(31)).get()};
//    Tester tc2{5, 17, uut->asyncCall(open, uint8_t(5), uint8_t(17)).get()};
//
//    for(int i = 0; i<10; i++)
//        tc1.runTest(uut);
//
//    for(int i = 0; i<5; i++)
//        tc2.runTest(uut);
//
//    for(int i = 0; i<7; i++)
//    {
//        tc1.runTest(uut);
//        tc2.runTest(uut);
//    }
//
//    tc1.close(uut);
//    tc2.close(uut);
//}

std::thread runInteropTests(std::shared_ptr<Rpc> uut)
{
    uut->provide(syms::echo, [](Rpc::Endpoint& h, const rpc::MethodHandle &self, const std::string &str, rpc::Call<std::string> cb) {
        return h.call(cb, str);
    });

    volatile bool locked = true;
    volatile int makeCnt = 0, delCnt = 0;

    uut->provide(syms::unlock, [&locked](Rpc::Endpoint& h, const rpc::MethodHandle &self, bool doIt)
    {
        if(doIt)
            locked = false;
    });

    uut->provide(syms::open, [&makeCnt, &delCnt](Rpc::Endpoint& h, const rpc::MethodHandle &self, uint8_t init, uint8_t modulus, rpc::Call<syms::Methods> cb)
    {
        struct State
        {
            decltype(delCnt) &dcnt;
            uint16_t x;
            const uint16_t mod;
            inline State(decltype(makeCnt) &makeCnt, decltype(delCnt) &delCnt, uint16_t x, uint16_t mod): 
                dcnt(delCnt), x(x), mod(mod)
            {
                makeCnt++;
            }

            inline ~State() {
                dcnt++;
            }

            syms::Read read;
        };

        auto state = std::make_shared<State>(makeCnt, delCnt, init, modulus);

        state->read = h.install([state](Rpc::Endpoint& h, const rpc::MethodHandle &self, uint32_t n, rpc::Call<std::vector<uint16_t>> cb)
        {
            std::vector<uint16_t> ret;
            ret.reserve(n);

            while(n--)
            {
                ret.push_back(state->x);
                state->x = state->x * state->x % state->mod;
            }

            return h.call(cb, ret);
        });

        auto close = h.install([state](Rpc::Endpoint& h, const rpc::MethodHandle &self)
        {
            if(auto err = h.uninstall(state->read))
                return err;

            return h.uninstall(self);
        });

        return h.call(cb, syms::Methods(close, state->read));
    });

    auto t = startServiceThread(uut);

    uut->unlock(false);

    runUnknownMethodLookupTest(uut);
    runEchoTest(uut);
    //    runStreamGeneratorTest(uut);

    uut->unlock(true);

    while(locked) std::this_thread::yield();

//    assert(makeCnt > 0);
    assert(delCnt == makeCnt);
    
    return t;
}

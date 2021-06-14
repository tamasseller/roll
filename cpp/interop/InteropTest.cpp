#include "InteropTest.h"

#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"
#include "RpcStreamReader.h"

#include <iostream>
#include <string>
#include <random>

namespace syms
{
    static constexpr auto echo = rpc::symbol<std::string, rpc::Call<std::string>>("echo"_ctstr);
    static constexpr auto nope = rpc::symbol<>("nope"_ctstr);
    static constexpr auto unlock = rpc::symbol<bool>("unlock"_ctstr);
    using Close = rpc::Call<>;
    using Read = rpc::Call<uint32_t, rpc::Call<std::list<uint16_t>>>;
    using Methods = std::tuple<Close, Read>;
    static constexpr auto open = rpc::symbol<uint8_t, uint8_t, rpc::Call<Methods>>("open"_ctstr);
}

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
    bool erred = false;

    try {
        auto nope = uut->lookup(syms::nope).get();
        (void)&nope;
    } 
    catch(const std::exception& e)
    {
        erred = true;
        assert(std::string(e.what()).find(rpc::Errors::noSuchSymbol) != std::string::npos);
    }

    assert(erred);
}

static inline void runEchoTest(std::shared_ptr<Rpc> uut)
{
    rpc::Call<std::string, rpc::Call<std::string>> echo = uut->lookup(syms::echo).get();
    auto key = generateUniqueKey();
    auto result = uut->asyncCall(echo, key);
    assert(result.get() == key);
}

static inline void runStreamGeneratorTest(std::shared_ptr<Rpc> uut)
{
    auto open = uut->lookup(syms::open).get();

    struct Tester
    {
        uint16_t state;
        const uint16_t mod;
        syms::Methods methods;
        
        void runTest(decltype(uut)& uut) 
        {
            int idx = 0;
            for(auto v: uut->asyncCall(std::get<1>(methods), 5).get())
            {
                idx++;
                assert(v == state);
                state = state * state % mod;
            }   
            assert(idx == 5);
        }

        void close(decltype(uut)& uut) {
            uut->simpleCall(std::get<0>(methods));
        }
    };

    Tester tc1{3, 31, uut->asyncCall(open, 3, 31).get()};
    Tester tc2{5, 17, uut->asyncCall(open, 5, 17).get()};

    for(int i = 0; i<10; i++)
        tc1.runTest(uut);

    for(int i = 0; i<5; i++)
        tc2.runTest(uut);

    for(int i = 0; i<7; i++)
    {
        tc1.runTest(uut);
        tc2.runTest(uut);
    }

    tc1.close(uut);
    tc2.close(uut);
}

std::thread runInteropTests(std::shared_ptr<Rpc> uut)
{
    uut->provide(syms::echo, [](Rpc::Endpoint& h, const rpc::MethodHandle &self, const auto &str, const auto &cb) {
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

    uut->simpleCall(uut->lookup(syms::unlock).get(), false);

    runUnknownMethodLookupTest(uut);
    runEchoTest(uut);
    runStreamGeneratorTest(uut);

    uut->simpleCall(uut->lookup(syms::unlock).get(), true);

    while(locked) std::this_thread::yield();

    assert(makeCnt > 0);
    assert(delCnt == makeCnt);
    
    return t;
}
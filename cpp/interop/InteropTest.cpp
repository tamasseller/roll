#include "InteropTest.h"

#include "RpcStlArray.h"
#include "RpcStlList.h"

#include "Contract.gen.h"

#include "RpcFdStreamAdapter.h"

#include "Tcp.h"

#include <iostream>
#include <random>

static constexpr uint16_t defaultInitial = 2;
static constexpr uint16_t defaultModulus = 19;

using Client = InteropTestContract::ClientProxy<rpc::FdStreamAdapter>;

template<class Target>
static inline auto startServiceThread(std::shared_ptr<Target> uut)
{
    return std::thread([uut]()
	{
        while(((rpc::FdStreamAdapter*)uut.get())->receive([&uut](auto message)
        {    
            if(auto a = message.access(); auto err = uut->process(a))
                std::cerr << "RPC SRV: " << err << std::endl;

            return true;
        }));
    });
}

struct Service: InteropTestContract::ServerProxy<Service, rpc::FdStreamAdapter>
{
	using Service::ServerProxy::ServerProxy;
	std::mutex m;
	std::condition_variable cv;
	volatile bool locked = true;
	volatile int makeCnt = 0, delCnt = 0;

	struct StreamSession: InteropTestStreamServerSession<StreamSession>
	{
		Service* srv;

		uint16_t x;
		const uint16_t mod;

		StreamSession(Service* srv, uint8_t initial, uint8_t modulus):
			srv(srv), x(initial), mod(modulus)
		{
			srv->makeCnt++;
		}

		~StreamSession() {
			srv->delCnt--;
		}

		void generate(uint32_t nData)
		{
			std::vector<uint16_t> ret;
			ret.reserve(nData);

			while(nData--)
			{
				ret.push_back(x);
				x = x * x % mod;
			}

			takeResult(srv, ret);
		}
	};

	std::pair<std::string, std::shared_ptr<StreamSession>> open(uint8_t initial, uint8_t modulus) {
		return {"asd", std::make_shared<Service::StreamSession>(this, initial, modulus)};
	}

	std::shared_ptr<StreamSession> openDefault() {
		return std::make_shared<Service::StreamSession>(this, defaultInitial, defaultModulus);
	}

	void unlock(bool doIt)
	{
		if(doIt)
		{
			std::unique_lock<std::mutex> l(m);
			locked = false;
			cv.notify_all();
		}
	}

	std::string echo(const std::string& str) {
		return std::move(str);
	}

	void wait()
	{
		for(std::unique_lock<std::mutex> l(m); locked;)
		{
			cv.wait(l);
		}
	}
};

void runInteropListener(int sock)
{
	auto uut = std::make_shared<Service>(sock, sock);
	auto t = startServiceThread(uut);

	uut->wait();

    assert(uut->makeCnt > 0);
    //assert(uut->delCnt == uut->makeCnt);

    closeNow(sock);
    t.join();
}

struct ClientStream: InteropTestStreamClientSession<ClientStream>
{
	uint16_t state = defaultInitial;
	const uint16_t mod = defaultModulus;

	int n = 0;
	std::mutex m;
	std::condition_variable cv;

	ClientStream() = default;
	ClientStream(uint16_t state, uint16_t mod): state(state), mod(mod) {}

	void takeResult(const std::vector<uint16_t> &data)
	{
		for(const auto& v: data)
		{
			assert(v == state);
			state = state * state % mod;
		}

		assert(data.size() == 5);

		{
			std::lock_guard _(m);
			n++;
			cv.notify_all();
		}
	}

	void wait(int n)
	{
		std::unique_lock<std::mutex> l(m);

		while(this->n != n)
		{
			cv.wait(l);
		}
	}
};

static inline void runUnknownMethodLookupTest(std::shared_ptr<Client> uut)
{
    static constexpr auto nope = rpc::symbol<>("nope"_ctstr);

	std::promise<bool> p;
	auto ret = p.get_future();

	const char* err = uut->lookup(nope, [p{std::move(p)}](auto&, bool done, auto) mutable { p.set_value(done); });
	assert(err == nullptr);

	bool failed = ret.get() == false;

    assert(failed);
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

static inline void runEchoTest(std::shared_ptr<Client> uut)
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

static inline void runStreamGeneratorTest(std::shared_ptr<Client> uut)
{
    auto s = std::make_shared<ClientStream>(3, 31);
    uut->open(s, uint8_t(3), uint8_t(31), [&s, &uut](std::string ret)
	{
    	assert(ret == "asd");

    	for(int i = 0; i < 3; i++)
		{
    		s->generate(uut, uint32_t(5));
		}
    });

    auto t = std::make_shared<ClientStream>();
    uut->openDefault(t, [&t, &uut]() {
   		t->generate(uut, uint32_t(5));
    });

    auto u = std::make_shared<ClientStream>();
    uut->openDefault(u).get();
    u->generate(uut, uint32_t(5));

    auto v = std::make_shared<ClientStream>(5, 17);
    auto str = uut->open<std::string>(v, uint8_t(5), uint8_t(17)).get();
    assert(str == "asd");
    v->generate(uut, uint32_t(5));

    s->wait(3);
    t->wait(1);
    u->wait(1);
    v->wait(1);
}

void runInteropTests(int sock)
{
	auto uut = std::make_shared<Client>(sock, sock);
	auto t = startServiceThread(uut);
	uut->unlock(false);

    runUnknownMethodLookupTest(uut);
    runEchoTest(uut);
    runStreamGeneratorTest(uut);

    uut->unlock(true);

    closeNow(sock);
    t.join();
}


#include "RpcEndpoint.h"
#include "RpcCTStr.h"
#include "RpcStlArray.h"
#include "RpcStlMap.h"
#include "RpcStlTuple.h"
#include "RpcSymbol.h"

#include "MockCoreAdapters.h"

#include "1test/Test.h"

#include <map>
#include <list>
#include <sstream>

struct MockMethodDictionary
{
    std::map<std::string, uint32_t> dict;

    class Query: std::stringstream
    {
        decltype(dict)& d;

        friend MockMethodDictionary;
        Query(decltype(dict)& d): d(d) {}

    public:
        auto &operator<<(char c) {
            return *((std::stringstream*)this) << c;
        }

        bool run(uint32_t &result) 
        {
            auto it = d.find(this->str());

            if(it == d.end())
                return false;

            result = it->second;
            return true;
        }
    };

    auto beginQuery() {
        return Query(dict);
    }

    bool addMapping(const char* name, uint32_t value) {
        return dict.insert({name, value}).second;
    }
};

struct MockIoEngine
{
    std::list<MockStream> sent;

    inline bool send(MockStream &&s) {
        sent.emplace_back(std::move(s));
        return true;
    }
};

struct MockLog
{
    static inline void write(const char* error) {
        MOCK(Log)::CALL(write).withStringParam(error);
    }
};

TEST_GROUP(Endpoint) 
{
    using Uut = rpc::Endpoint<
        MockLog,
        MockMethodDictionary, 
        MockIoEngine, 
        MockStreamWriterFactory, 
        MockSmartPointer, 
        MockRegistry
    >;

    static inline void executeLoopback(Uut& uut) 
    {
        auto msg = rpc::move(uut.sent.front());
        uut.sent.pop_front();
        auto loopback = msg.access();
        CHECK(uut.execute(loopback));
    }
};

TEST(Endpoint, Hello)
{
    Uut uut;
    uut.init();

    int n = 0;
    rpc::Call<std::string> cb = uut.install([&n](const std::string &str) {
        CHECK(str == "hello");
        n++;
    });

    CHECK(uut.call(cb, "hello"));
    executeLoopback(uut);
    CHECK(n == 1);
}

TEST(Endpoint, ProvideRequire)
{
    Uut uut;
    uut.init();

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>(rpc::CTStr("symbol"));

    CHECK(uut.provide(sym, [&uut](auto x, auto callback) {}));

    bool done = false;
    CHECK(uut.lookup(sym, [&done](bool lookupSucced, auto sayHello)
    {
        CHECK(lookupSucced);

        (rpc::Call<uint32_t, rpc::Call<std::string>>)sayHello;
        CHECK(sayHello.id == 1);
        done = true;
    }));

    for(int i = 0; i < 2; i++)
    executeLoopback(uut);
    CHECK(done);
}

TEST(Endpoint, ExecuteRemoteWithCallback)
{
    Uut uut;
    uut.init();

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>(rpc::CTStr("say-hello"));

    CHECK(uut.provide(sym, [&uut](auto x, auto callback)
    {
        for(auto i = 0u; i < x; i++)
            CHECK(uut.call(callback, "hello"));
    }));

    bool done = false;
    CHECK(uut.lookup(sym, [&done, &uut](bool lookupSucceded, auto sayHello)
    {
        CHECK(lookupSucceded);

        int n = 0;
        rpc::Call<std::string> cb = uut.install([&n](const std::string &str) {
            CHECK(str == "hello");
            n++;
        });
    
        CHECK(uut.call(sayHello, 3, cb));

        for(int i = 0; i < 4; i++)
            executeLoopback(uut);

        CHECK(n == 3);
        done = true;
    }));

    for(int i=0; i<2; i++)
        executeLoopback(uut);

    CHECK(done);
}
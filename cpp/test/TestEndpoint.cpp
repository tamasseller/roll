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

    int failAt = 0;

    inline bool send(MockStream &&s) 
    {
        if(failAt > 0)
        {
            if(--failAt == 0)
                return false;
        }
        
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
        CHECK(uut.process(loopback));
    }
};

TEST(Endpoint, Hello)
{
    Uut uut;
    CHECK(uut.init());

    int n = 0;
    rpc::Call<std::string> cb = uut.install([&n](Uut &uut, rpc::MethodHandle h, const std::string &str) {
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
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>(rpc::CTStr("symbol"));

    CHECK(uut.provide(sym, [](Uut &uut, rpc::MethodHandle id, auto x, auto callback) {}));

    bool done = false;
    CHECK(uut.lookup(sym, [&done](Uut &uut, bool lookupSucced, auto sayHello)
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

TEST(Endpoint, DoubleProvide)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<long>(rpc::CTStr("you-only-provide-once"));
    CHECK(uut.provide(sym, [](auto &uut, auto id, auto x){}));
    CHECK(!uut.provide(sym, [](auto &uut, auto id, auto x){}));
}

TEST(Endpoint, DoubleInit)
{
    Uut uut;
    CHECK(uut.init());
    CHECK(!uut.init());
}

TEST(Endpoint, ExecuteRemoteWithCallback)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>(rpc::CTStr("say-hello"));

    CHECK(uut.provide(sym, [](auto &uut, auto id, auto x, auto callback)
    {
        for(auto i = 0u; i < x; i++)
            CHECK(uut.call(callback, "hello"));
    }));

    bool done = false;
    CHECK(uut.lookup(sym, [&done](Uut& uut, bool lookupSucceded, auto sayHello)
    {
        CHECK(lookupSucceded);

        int n = 0;
        rpc::Call<std::string> cb = uut.install([&n](Uut &uut, rpc::MethodHandle id, const std::string &str) {
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

TEST(Endpoint, LookupTotallyUnknownMethod)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<uint32_t>(rpc::CTStr("non-existent"));

    bool done = false;
    CHECK(uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto)
    {
        CHECK(!lookupSucceded);
        done = true;
    }));  

    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::unknownMethodRequested);

    for(int i=0; i<2; i++)
        executeLoopback(uut);

    CHECK(done);
}

TEST(Endpoint, LookupMethodWithMistmatchedSignatureUnknownMethod)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto defsym = rpc::symbol<std::string>(rpc::CTStr("almost"));

    CHECK(uut.provide(defsym, [](auto &uut, auto id, auto x){}));

    constexpr auto lookupsym = rpc::symbol<int>(rpc::CTStr("almost"));

    bool done = false;
    CHECK(uut.lookup(lookupsym, [&done](auto &uut, bool lookupSucceded, auto)
    {
        CHECK(!lookupSucceded);
        done = true;
    }));  

    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::unknownMethodRequested);

    for(int i=0; i<2; i++)
        executeLoopback(uut);

    CHECK(done);
}

TEST(Endpoint, FailToSendLookup)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>(rpc::CTStr("dont-care"));

    uut.failAt = 1;

    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::couldNotSendLookupMessage);

    CHECK(!uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }));  

    CHECK(!done);
}

TEST(Endpoint, FailToSendLookupResponse)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>(rpc::CTStr("dont-care"));

    uut.failAt = 2;

    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::unknownMethodRequested);
    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::couldNotSendMessage);
    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::failedToReplyToLookup);

    CHECK(uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }));
    executeLoopback(uut);

    CHECK(!done);
}


TEST(Endpoint, FailToCreateLookup)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>(rpc::CTStr("dont-care"));

    MockStreamWriterFactory::failAt = 1;

    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::couldNotCreateLookupMessage);

    CHECK(!uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }));  

    CHECK(!done);
}

TEST(Endpoint, FailToCreateLookupResponse)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>(rpc::CTStr("dont-care"));

    MockStreamWriterFactory::failAt = 2;

    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::unknownMethodRequested);
    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::couldNotCreateMessage);
    MOCK(Log)::EXPECT(write).withStringParam(rpc::Errors::failedToReplyToLookup);

    CHECK(uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }));
    executeLoopback(uut);

    CHECK(!done);
}


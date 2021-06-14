#include "RpcEndpoint.h"
#include "RpcCTStr.h"
#include "RpcStlArray.h"
#include "RpcStlMap.h"
#include "RpcStlTuple.h"
#include "RpcSymbol.h"

#include "MockCoreAdapters.h"
#include "TestCallIdAccessor.h"

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

        bool run(MockMethodDictionary&, uint32_t &result) 
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

    bool removeMapping(const char* name, uint32_t &value) 
    {
        auto it = dict.find(name);

        if(it != dict.end())
        {
            value = it->second;
            dict.erase(it);
            return true;
        }

        return false;
    }

    bool removeMapping(uint32_t value) 
    {
        for(auto it = dict.begin(); it != dict.end(); it++)
        {
            if(it->second == value)
            {
                dict.erase(it);
                return true;
            }
        }

        return false;
    }
};

struct Uut: rpc::Endpoint<
        MockSmartPointer, 
        MockRegistry,
        MockMethodDictionary, 
        Uut,
        MockStreamWriterFactory
> {
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

TEST_GROUP(Endpoint), rpc::CallIdTestAccessor
{
    static inline void executeLoopback(Uut::Endpoint& ep, const char* exp = nullptr) 
    {
        auto &uut = (Uut&)ep;
        auto msg = rpc::move(uut.sent.front());
        uut.sent.pop_front();
        auto loopback = msg.access();
        CHECK(exp == uut.process(loopback));
    }
};

TEST(Endpoint, Hello)
{
    Uut uut;
    CHECK(uut.init());

    int n = 0;
    rpc::Call<std::string> cb = uut.install([&n](Uut::Endpoint& uut, const rpc::MethodHandle &h, const std::string &str) {
        CHECK(str == "hello");
        n++;
    });

    CHECK(nullptr == uut.call(cb, "hello"));
    executeLoopback(uut);
    CHECK(n == 1);
}

TEST(Endpoint, NotHello)
{
    Uut uut;
    CHECK(uut.init());
    rpc::Call<std::string> cb = uut.install([](Uut::Endpoint& uut, const rpc::MethodHandle &h, const std::string &str) {});
    CHECK(nullptr == uut.uninstall(cb));
    CHECK(uut.uninstall(cb) == rpc::Errors::methodNotFound);
}

TEST(Endpoint, ProvideRequire)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>("symbol"_ctstr);

    CHECK(nullptr == uut.provide(sym, [](Uut::Endpoint& uut, const rpc::MethodHandle &id, auto x, auto callback) {}));

    bool done = false;
    CHECK(nullptr == uut.lookup(sym, [&done](Uut::Endpoint& uut, bool lookupSucced, auto sayHello)
    {
        CHECK(lookupSucced);

        (rpc::Call<uint32_t, rpc::Call<std::string>>)sayHello;
        CHECK(getId(sayHello) == 1);
        done = true;
    }));

    for(int i = 0; i < 2; i++)
        executeLoopback(uut);

    CHECK(done);
}

TEST(Endpoint, ProvideDiscardRequire)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>("symbol"_ctstr);
    CHECK(uut.discard(sym) == rpc::Errors::noSuchSymbol);

    CHECK(nullptr == uut.provide(sym, [](Uut::Endpoint& uut, const rpc::MethodHandle &id, auto x, auto callback) {}));
    CHECK(nullptr == uut.discard(sym));

    bool done = false;
    CHECK(nullptr == uut.lookup(sym, [&done](Uut::Endpoint& uut, bool lookupSucced, auto)
    {
        CHECK(!lookupSucced);
        done = true;
    }));

    executeLoopback(uut, rpc::Errors::unknownMethodRequested);
    executeLoopback(uut);

    CHECK(uut.discard(sym) == rpc::Errors::noSuchSymbol);

    CHECK(done);
}

TEST(Endpoint, ProvideRequireRemovedFromCall)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<>("symbol"_ctstr);
    CHECK(nullptr == uut.provide(sym, [](Uut::Endpoint& uut, const rpc::MethodHandle &id) {
        uut.uninstall(id);
    }));
   
    bool done1 = false;
    CHECK(nullptr == uut.lookup(sym, [&done1](Uut::Endpoint& uut, bool lookupSucced, auto result)
    {
        CHECK(lookupSucced);
        CHECK(nullptr == uut.call(result));
        done1 = true;
    }));

    for(int i = 0; i < 2; i++)
        executeLoopback(uut);

    CHECK(done1);

    executeLoopback(uut);
    CHECK(uut.discard(sym) == rpc::Errors::noSuchSymbol);

    bool done2 = false;
    CHECK(nullptr == uut.lookup(sym, [&done2](Uut::Endpoint& uut, bool lookupSucced, auto)
    {
        CHECK(!lookupSucced);
        done2 = true;
    }));

    executeLoopback(uut, rpc::Errors::unknownMethodRequested);
    executeLoopback(uut);

    CHECK(done2);
}

TEST(Endpoint, DoubleProvide)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto sym = rpc::symbol<long>("you-only-provide-once"_ctstr);
    CHECK(nullptr == uut.provide(sym, [](auto &uut, auto id, auto x){}));
    CHECK(rpc::Errors::symbolAlreadyExported == uut.provide(sym, [](auto &uut, auto id, auto x){}));
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

    constexpr auto sym = rpc::symbol<uint32_t, rpc::Call<std::string>>("say-hello"_ctstr);

    CHECK(nullptr == uut.provide(sym, [](auto &uut, auto id, auto x, auto callback)
    {
        for(auto i = 0u; i < x; i++)
            CHECK(nullptr == uut.call(callback, "hello"));
    }));

    bool done = false;
    CHECK(nullptr == uut.lookup(sym, [&done](Uut::Endpoint& uut, bool lookupSucceded, auto sayHello)
    {
        CHECK(lookupSucceded);

        int n = 0;
        rpc::Call<std::string> cb = uut.install([&n](Uut::Endpoint &uut, const rpc::MethodHandle &id, const std::string &str) {
            CHECK(str == "hello");
            n++;
        });
    
        CHECK(nullptr == uut.call(sayHello, 3, cb));

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

    constexpr auto sym = rpc::symbol<uint32_t>("non-existent"_ctstr);

    bool done = false;
    CHECK(nullptr == uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto)
    {
        CHECK(!lookupSucceded);
        done = true;
    }));  

    executeLoopback(uut, rpc::Errors::unknownMethodRequested);
    executeLoopback(uut);

    CHECK(done);
}

TEST(Endpoint, LookupMethodWithMistmatchedSignatureUnknownMethod)
{
    Uut uut;
    CHECK(uut.init());

    constexpr auto defsym = rpc::symbol<std::string>("almost"_ctstr);

    CHECK(nullptr == uut.provide(defsym, [](auto &uut, auto id, auto x){}));

    constexpr auto lookupsym = rpc::symbol<int>("almost"_ctstr);

    bool done = false;
    CHECK(nullptr == uut.lookup(lookupsym, [&done](auto &uut, bool lookupSucceded, auto)
    {
        CHECK(!lookupSucceded);
        done = true;
    }));  

    executeLoopback(uut, rpc::Errors::unknownMethodRequested);
    executeLoopback(uut);

    CHECK(done);
}

TEST(Endpoint, FailToSendLookup)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>("dont-care"_ctstr);

    uut.failAt = 1;

    CHECK(uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }) 
        == rpc::Errors::couldNotSendLookupMessage);

    CHECK(!done);
}

TEST(Endpoint, FailToSendLookupResponse)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>("dont-care"_ctstr);

    uut.failAt = 2;

    CHECK(nullptr == uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }));

    executeLoopback(uut, rpc::Errors::couldNotSendMessage);

    CHECK(!done);
}

TEST(Endpoint, FailToCreateLookup)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>("dont-care"_ctstr);

    MockStreamWriterFactory::failAt = 1;

    CHECK(uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; })
        == rpc::Errors::couldNotCreateLookupMessage);  

    CHECK(!done);
}

TEST(Endpoint, FailToCreateLookupResponse)
{
    Uut uut;
    CHECK(uut.init());
    bool done = false;

    constexpr auto sym = rpc::symbol<>("dont-care"_ctstr);

    MockStreamWriterFactory::failAt = 2;

    CHECK(nullptr == uut.lookup(sym, [&done](auto &uut, bool lookupSucceded, auto sayHello){ done = true; }));
    executeLoopback(uut, rpc::Errors::couldNotCreateMessage);

    CHECK(!done);
}

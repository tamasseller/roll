#include "RpcEndpoint.h"
#include "RpcCTStr.h"
#include "RpcStlArray.h"

#include "MockCoreAdapters.h"

#include "1test/Test.h"

#include <map>
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

        bool run(uint32_t result) 
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
};

struct MockIoEngine
{
    MockStream sent;

    inline bool send(MockStream &&s) {
        sent = std::move(s);
        return true;
    }
};

TEST_GROUP(Endpoint) 
{
    using Uut = rpc::Endpoint<
        MockMethodDictionary, 
        MockIoEngine, 
        MockStreamWriterFactory, 
        MockSmartPointer, 
        MockRegistry
    >;
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

    auto a = uut.sent.access();
    CHECK(uut.execute(a));

    CHECK(n == 1);
}
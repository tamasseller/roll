#include "RpcCore.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"

#include "MockCoreAdapters.h"

#include "1test/Test.h"

TEST_GROUP(Core)  {};

TEST(Core, AddCallAt)
{
    rpc::Core<MockStreamWriterFactory, MockSmartPointer, MockRegistry> core;
    bool buildOk;
    auto call = core.buildCall<std::string>(buildOk, 69, std::string("asdqwe"));
    CHECK(buildOk);

    bool done = false;

    auto a = call.access();
    CHECK(!core.execute(a));
    CHECK(!done);

    CHECK(core.addCallAt<std::string>(69, [&done](rpc::MethodHandle id, std::string str)
    {
        CHECK(str == "asdqwe");
        done = true;
    }));

    CHECK(!done);

    auto b = call.access();
    CHECK(core.execute(b));
    CHECK(done);

    done = false;
    auto c = call.access();
    CHECK(core.execute(c));
    CHECK(done);

    CHECK(!core.addCallAt<std::string>(69, [&done](rpc::MethodHandle id, std::string str) { CHECK(false); }));

    done = false;
    auto d = call.access();
    CHECK(core.execute(d));
    CHECK(done);

    CHECK(core.removeCall(69));

    done = false;
    auto e = call.access();
    CHECK(!core.execute(e));
    CHECK(!done);

    CHECK(!core.removeCall(69));
}

TEST(Core, Truncate)
{
    rpc::Core<MockStreamWriterFactory, MockSmartPointer, MockRegistry> core;

    bool done = false;
    CHECK(core.addCallAt<std::list<std::vector<char>>>(69, [&done](rpc::MethodHandle id, auto str)
    {
        CHECK(str == std::list<std::vector<char>>{{'a', 's', 'd'}, {'q', 'w', 'e'}});
        CHECK(!done);
        done = true;
    }));

    for(int i = 0; ; i++)
    {
        bool buildOk;
        auto call = core.buildCall<std::vector<std::string>>(buildOk, 69, std::vector<std::string>{"asd", "qwe"});
        CHECK(buildOk);

        if(call.truncateAt(i))
        {
            auto a = call.access();
            CHECK(!core.execute(a));
        }
        else
        {
            auto a = call.access();
            CHECK(core.execute(a));
            break;
        }
    }

    CHECK(done);
}

TEST(Core, GenericInsert)
{
    rpc::Core<MockStreamWriterFactory, MockSmartPointer, MockRegistry> core;

    bool a = false;
    int b = 0;

    core.addCallAt(0, [](rpc::MethodHandle id){});
    auto id1 = core.add([&a](auto id) { a = true; });
    auto id2 = core.add<int, int>([&b](rpc::MethodHandle id, int x, int y) { b = x + y; });

    bool build1ok;
    auto call1 = core.buildCall(build1ok, id1);
    CHECK(build1ok);

    auto r1 = call1.access();
    CHECK(core.execute(r1));
    CHECK(a == true);

    bool build2ok;
    auto call2 = core.buildCall<int, int>(build2ok, id2, 1, 2);
    CHECK(build2ok);
    auto r2 = call2.access();
    CHECK(core.execute(r2));
    CHECK(b == 3);
}

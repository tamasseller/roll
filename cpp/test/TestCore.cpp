#include "RpcCore.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"

#include "MockStream.h"

#include "1test/Test.h"

#include <map>

TEST_GROUP(Core) 
{
    struct SelfContainedStreamWriter: MockStream, MockStream::Accessor {
        SelfContainedStreamWriter(size_t s): MockStream(s), MockStream::Accessor(this->access()) {}
    };

    struct MockStreamWriterFactory
    {
        using Accessor = MockStream::Accessor;

        static inline auto build(size_t s) { 
            return SelfContainedStreamWriter(s); 
        }

        decltype(auto)static inline done(SelfContainedStreamWriter &&w) { 
            return static_cast<MockStream&&>(w); 
        }
    };

    template<class K, class V>
    class Registry
    {
        std::map<K, V> lookupTable;

    public:
        inline bool remove(const K& k) 
        {
            auto it = lookupTable.find(k);

            if(it == lookupTable.end())
                return false;

            lookupTable.erase(it);
            return true;
        }

        inline bool add(const K& k, V&& v) 
        {
            auto it = lookupTable.find(k);

            if(it != lookupTable.end())
                return false;

            return lookupTable.emplace(k, std::move(v)).second;
        }

        inline V* find(const K& k, bool &ok) 
        {
            auto it = lookupTable.find(k);

            if(it == lookupTable.end())
            {
                ok = false;
                return nullptr;
            }
            
            ok = true;
            return &it->second;
        }
    };

    template<class T>
    struct Pointer: std::unique_ptr<T> 
    {
        Pointer(std::unique_ptr<T> &&v): std::unique_ptr<T>(std::move(v)) {}

        template<class U, class... Args>
        static inline Pointer make(Args&&... args) {
            return Pointer(std::unique_ptr<T>(new U(std::forward<Args>(args)...)));
        }
    };
};

TEST(Core, AddCallAt)
{
    rpc::Core<MockStreamWriterFactory, Pointer, Registry> core;
    auto call = core.buildCall<std::string>(69, std::string("asdqwe"));

    bool done = false;

    auto a = call.access();
    CHECK(!core.execute(a));
    CHECK(!done);

    CHECK(core.addCallAt<std::string>(69, [&done](std::string str)
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

    CHECK(!core.addCallAt<std::string>(69, [&done](std::string str) { CHECK(false); }));

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
    rpc::Core<MockStreamWriterFactory, Pointer, Registry> core;

    bool done = false;
    CHECK(core.addCallAt<std::list<std::vector<char>>>(69, [&done](auto str)
    {
        CHECK(str == std::list<std::vector<char>>{{'a', 's', 'd'}, {'q', 'w', 'e'}});
        CHECK(!done);
        done = true;
    }));

    for(int i = 0; ; i++)
    {
        auto call = core.buildCall<std::vector<std::string>>(69, std::vector<std::string>{"asd", "qwe"});

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
    rpc::Core<MockStreamWriterFactory, Pointer, Registry> core;

    bool a = false;
    int b = 0;

    core.addCallAt(0, [](){});
    auto id1 = core.add([&a]() { a = true; });
    auto id2 = core.add<int, int>([&b](int x, int y) { b = x + y; });

    auto call1 = core.buildCall(id1);
    auto r1 = call1.access();
    CHECK(core.execute(r1));
    CHECK(a == true);

    auto call2 = core.buildCall<int, int>(id2, 1, 2);
    auto r2 = call2.access();
    CHECK(core.execute(r2));
    CHECK(b == 3);
}
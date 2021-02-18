#include "RpcSerdes.h"
#include "RpcStlMap.h"
#include "RpcStlSet.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"
#include "RpcStlArray.h"

#include "pet/1test/Test.h"

#include <memory>
#include <string.h>

TEST_GROUP(SerDes) 
{
    using a = std::initializer_list<int>;

    class MockStream
    {
        std::unique_ptr<char[]> buffer;
        char *const end;

        class Accessor
        {
            friend MockStream;
            char *ptr, *const end;
            inline Accessor(char* ptr, char* end): ptr(ptr), end(end) {}

        public:
            template<class T>
            bool write(const T& v)
            {
                static constexpr auto size = sizeof(T);
                if(size <= end - ptr)
                {
                    memcpy(ptr, &v, size);
                    ptr += size;
                    return true;
                }

                return false;
            }

            template<class T>
            bool read(T& v)
            {
                static constexpr auto size = sizeof(T);
                if(size <= end - ptr)
                {
                    memcpy(&v, ptr, size);
                    ptr += size;
                    return true;
                }

                return false;
            }

            template<class T>
            bool skip(size_t size)
            {
                if(size <= end - ptr)
                {
                    ptr += size;
                    return true;
                }

                return false;
            }
        };

    public:
        inline auto access() {
            return Accessor(buffer.get(), end);
        }

        inline MockStream(size_t size): buffer(new char[size]), end(buffer.get() + size) {};
    };

    template<class... C>
    void test(C&&... c)
    {
        MockStream stream(determineSize<C...>(std::forward<C>(c)...));
        auto a = stream.access();
        CHECK(serialize<C...>(a, std::forward<C>(c)...));
        MOCK(SerDes)::EXPECT(test);
        CHECK(!a.write('\0'));

        auto b = stream.access();
        bool deserOk = deserialize<C...>(b, [exp{std::make_tuple<C...>(std::forward<C>(c)...)}](auto&&... args){
            CHECK(std::make_tuple(args...) == exp);
            MOCK(SerDes)::CALL(test);
        });
        CHECK(deserOk);
    }
};

TEST(SerDes, Void) {
    test();
}

TEST(SerDes, Int) {
    test(int(1));
}

TEST(SerDes, Ints) {
     test(int(1), int(2));
}

TEST(SerDes, Mixed) {
     test(int(3), short(4), (unsigned char)5);
}

TEST(SerDes, Pair) {
     test(std::make_pair(char(6), long(7)));
}

TEST(SerDes, Tuple) {
     test(std::make_tuple(int(8), short(9), char(10)));
}

TEST(SerDes, IntList) {
    test(std::list<int>{11, 12, 13});
}

TEST(SerDes, ULongDeque) {
    test(std::deque<unsigned long>{123456ul, 234567ul, 3456789ul});
}

TEST(SerDes, UshortForwardList) {
    test(std::forward_list<unsigned short>{123, 231, 312});
}

TEST(SerDes, ShortVector) {
    test(std::vector<short>{14, 15});
}

TEST(SerDes, EmptyLongVector) {
    test(std::vector<long>{});
}

TEST(SerDes, String) {
    test(std::string("Hi!"));
}

TEST(SerDes, CharSet) {
    test(std::set<char>{'a', 'b', 'c'});
}

TEST(SerDes, CharMultiSet) {
    test(std::set<char>{'x', 'x', 'x'});
}

TEST(SerDes, IntToCharMap) {
    test(std::map<int, char>{{1, 'a'}, {2, 'b'}, {3, 'c'}, {4, 'a'}});
}

TEST(SerDes, CharToIntMultimap) {
    test(std::multimap<char, int>{{'a', 1}, {'b', 2}, {'c', 3}, {'a', 4}});
}

TEST(SerDes, CharUnorderedSet) {
    test(std::unordered_set<char>{'a', 'b', 'c'});
}

TEST(SerDes, CharUnorderedMultiSet) {
    test(std::unordered_set<char>{'x', 'x', 'x'});
}

TEST(SerDes, IntToCharUnorderedMap) {
    test(std::unordered_map<int, char>{{1, 'a'}, {2, 'b'}, {3, 'c'}, {4, 'a'}});
}

TEST(SerDes, CharToIntUnorderedMultimap) {
    test(std::unordered_multimap<char, int>{{'a', 1}, {'b', 2}, {'c', 3}, {'a', 4}});
}

TEST(SerDes, MultilevelDocumentStructure) {
    test(std::tuple<std::set<std::string>, std::map<std::pair<std::string, std::string>, int>, std::list<std::vector<std::string>>> (
        {"alpha", "beta", "delta", "gamma", "epsilon"},
        {
            {{"alpha", "beta"}, 1},
            {{"beta", "gamma"}, 2},
            {{"alpha", "delta"}, 3},
            {{"delta", "gamma"}, 4},
            {{"gamma", "epsilon"}, 5}
        },
        {
            {"alpha", "beta", "gamma", "beta", "alpha"},
            {"alpha", "delta", "gamma", "epsilon"}
        }
    ));
}

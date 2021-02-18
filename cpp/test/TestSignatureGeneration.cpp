#include "RpcSerdes.h"
#include "RpcStlMap.h"
#include "RpcStlSet.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"
#include "RpcStlArray.h"

#include "pet/1test/Test.h"

#include <sstream>

TEST_GROUP(SignatureGenerator) 
{
    template<class... T>
    std::string sgn()
    {
        std::stringstream ss;
        writeSignature<T...>(ss);
        return ss.str();
    }
};

TEST(SignatureGenerator, VoidOfNothing) {
    CHECK(sgn<>() == "()");
}

TEST(SignatureGenerator, VoidOfInt) {
    CHECK(sgn<int>() == "(i4)");
}

TEST(SignatureGenerator, VoidOfUint) {
    CHECK(sgn<unsigned int>() == "(u4)");
}

TEST(SignatureGenerator, VoidOfShort) {
    CHECK(sgn<short>() == "(i2)");
}

TEST(SignatureGenerator, VoidOfUshort) {
    CHECK(sgn<unsigned short>() == "(u2)");
}

TEST(SignatureGenerator, VoidOfChar) {
    CHECK(sgn<char>() == "(i1)");
}

TEST(SignatureGenerator, VoidOfUchar) {
    CHECK(sgn<unsigned char>() == "(u1)");
}

TEST(SignatureGenerator, VoidOfLong) {
    CHECK(sgn<long>() == "(i8)");
}

TEST(SignatureGenerator, VoidOfUlong) {
    CHECK(sgn<unsigned long long>() == "(u8)");
}

TEST(SignatureGenerator, VoidOfBool) {
    CHECK(sgn<bool>() == "(b)");
}

TEST(SignatureGenerator, VoidOfTwoInts) {
    CHECK(sgn<int, int>() == "(i4,i4)");
}

TEST(SignatureGenerator, VoidOfTwoFourLongs) {
    CHECK(sgn<long, unsigned long, long long, unsigned long long>() == "(i8,u8,i8,u8)");
}

TEST(SignatureGenerator, VoidOfAllPrimitives) {
    CHECK(sgn<bool, char, unsigned char, short, unsigned short, int, unsigned int, long, unsigned long>() 
        == "(b,i1,u1,i2,u2,i4,u4,i8,u8)");
}

TEST(SignatureGenerator, VoidOfBitVector) {
    CHECK(sgn<std::vector<bool>>() == "([b])");
}

TEST(SignatureGenerator, VoidOfIntList) {
    CHECK(sgn<std::list<int>>() == "([i4])");
}

TEST(SignatureGenerator, VoidOfIntVectorAndUlongList) {
    CHECK(sgn<std::vector<int>, std::list<unsigned long>>() == "([i4],[u8])");
}

TEST(SignatureGenerator, IntOfNothing) {
    CHECK(sgn<RpcCall<int>>() == "((i4))");
}

TEST(SignatureGenerator, IntListOfByteVector) {
    CHECK(sgn<std::vector<unsigned char>, RpcCall<std::list<int>>>() == "([u1],([i4]))");
}

TEST(SignatureGenerator, VoidOfIntCharPair) {
    CHECK(sgn<std::pair<int, char>>() == "({i4,i1})");
}

TEST(SignatureGenerator, VoidOfUshort3tuple) {
    CHECK(sgn<std::tuple<unsigned short, unsigned short, unsigned short>>() == "({u2,u2,u2})");
}

TEST(SignatureGenerator, VoidOfByteVectorStringPairList) {
    CHECK(sgn<std::list<std::pair<std::vector<unsigned char>, std::string>>>() == "([{[u1],[i1]}])");
}

TEST(SignatureGenerator, StringToIntMultimapOfIntToStringMap) {
    CHECK(sgn<std::map<int, std::string>, RpcCall<std::multimap<std::string, int>>>() == "([{i4,[i1]}],([{[i1],i4}]))");
}

TEST(SignatureGenerator, CharToIntMapOfCharSet) {
    CHECK(sgn<std::set<char>, RpcCall<std::map<char,int>>>() == "([i1],([{i1,i4}]))");
}

#ifndef COMMON_RPCTYPEINFO_H_
#define COMMON_RPCTYPEINFO_H_

template<class C> struct RpcTypeInfo {};

static_assert(sizeof(bool) == 1);
template<> struct RpcTypeInfo<bool> { static constexpr const char* name = "bool"; };

static_assert(sizeof(char) == 1);
template<> struct RpcTypeInfo<char> { static constexpr const char* name = "i8"; };
template<> struct RpcTypeInfo<signed char> { static constexpr const char* name = "i8"; };
template<> struct RpcTypeInfo<unsigned char> { static constexpr const char* name = "u8"; };

static_assert(sizeof(short) == 2);
template<> struct RpcTypeInfo<signed short> { static constexpr const char* name = "i16"; };
template<> struct RpcTypeInfo<unsigned short> { static constexpr const char* name = "u16"; };

static_assert(sizeof(int) == 4);
template<> struct RpcTypeInfo<signed int> { static constexpr const char* name = "i32"; };
template<> struct RpcTypeInfo<unsigned int> { static constexpr const char* name = "u32"; };

static_assert(sizeof(long) == 8);
template<> struct RpcTypeInfo<signed long> { static constexpr const char* name = "i64"; };
template<> struct RpcTypeInfo<unsigned long> { static constexpr const char* name = "u64"; };

static_assert(sizeof(long long) == 8);
template<> struct RpcTypeInfo<signed long long> { static constexpr const char* name = "i64"; };
template<> struct RpcTypeInfo<unsigned long long> { static constexpr const char* name = "u64"; };

template<> struct RpcTypeInfo<const char*> { static constexpr const char* name = "str"; };

#endif /* COMMON_RPCTYPEINFO_H_ */

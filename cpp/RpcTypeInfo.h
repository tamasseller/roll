#ifndef _RPCTYPEINFO_H_
#define _RPCTYPEINFO_H_

#include "RpcSignatureGenerator.h"
#include "RpcVarInt.h"

#include <stddef.h>
#include <stdint.h>

namespace rpc {

template<> struct TypeInfo<bool> { 
	static_assert(sizeof(bool) == 1);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "b"; }
	template<class S> static inline bool write(S& s, const bool &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, bool &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(1); }
	static constexpr inline size_t size(...) { return 1; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<char> { 
	static_assert(sizeof(char) == 1);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "i1"; }
	template<class S> static inline bool write(S& s, const char &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, char &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(1); }
	static constexpr inline size_t size(...) { return 1; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<signed char>: TypeInfo<char> {};

template<> struct TypeInfo<unsigned char> 
{ 
	static_assert(sizeof(unsigned char) == 1);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "u1"; }
	template<class S> static inline bool write(S& s, const unsigned char &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned char &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(1); }
	static constexpr inline size_t size(...) { return 1; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<signed short> 
{
	static_assert(sizeof(signed short) == 2);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "i2"; }
	template<class S> static inline bool write(S& s, const signed short &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, signed short &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(2); }
	static constexpr inline size_t size(...) { return 2; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<unsigned short> 
{
	static_assert(sizeof(unsigned short) == 2);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "u2"; }
	template<class S> static inline bool write(S& s, const unsigned short &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned short &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(2); }
	static constexpr inline size_t size(...) { return 2; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<signed int> 
{
	static_assert(sizeof(signed int) == 4);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "i4"; }
	template<class S> static inline bool write(S& s, const signed int &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, signed int &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(4); }
	static constexpr inline size_t size(...) { return 4; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<unsigned int> 
{ 
	static_assert(sizeof(unsigned int) == 4);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "u4"; }
	template<class S> static inline bool write(S& s, const unsigned int &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned int &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(4); }
	static constexpr inline size_t size(...) { return 4; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<signed long> 
{
	static_assert(sizeof(signed long) == 8);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "i8"; }
	template<class S> static inline bool write(S& s, const signed long &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, signed long &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(8); }
	static constexpr inline size_t size(...) { return 8; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<signed long long> : TypeInfo<signed long> {};

template<> struct TypeInfo<unsigned long> 
{ 
	static_assert(sizeof(unsigned long) == 8);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "u8"; }
	template<class S> static inline bool write(S& s, const unsigned long &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned long &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(8); }
	static constexpr inline size_t size(...) { return 8; }
	static constexpr inline bool isConstSize() { return true; }
};

template<> struct TypeInfo<unsigned long long> : TypeInfo<unsigned long> {};

template<class... Args>
struct Call {
	uint32_t id = -1u;
};

template<class... Args> struct TypeInfo<Call<Args...>> 
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return writeSignature<Args...>(s); }
	template<class S> static inline bool write(S& s, const Call<Args...> &v) { return VarUint4::write(s, v.id); }
	template<class S> static inline bool read(S& s, Call<Args...> &v) { return VarUint4::read(s, v.id); }
	template<class S> static inline bool skip(S& s) { return VarUint4::skip(s); }
	static constexpr inline size_t size(const Call<Args...> &v) { return VarUint4::size(v.id); }
	static constexpr inline bool isConstSize() { return false; }
};

template<class T> struct CollectionTypeBase
{ 
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { 
		return TypeInfo<T>::writeName(s << "[") << "]";
	}

	template<class S> static inline bool skip(S& s)
    {
        uint32_t count;
        if(!VarUint4::read(s, count))
            return false;

        while(count--)
            if(!TypeInfo<T>::skip(s))
                return false;

        return true;
    }

	static constexpr inline bool isConstSize() { return false; }
};

template<class T> struct StlCompatibleCollectionTypeBase: CollectionTypeBase<T>
{ 
    template<class C> static constexpr inline size_t size(const C& v) 
    {
        size_t contentSize = 0;
        uint32_t count = 0;

        if constexpr(TypeInfo<T>::isConstSize())
        {
            count = v.size();
            contentSize = count ? (count * TypeInfo<T>::size(*v.begin())) : 0;
        }
        else
        {
            for(const auto &x: v)
            {
                contentSize += TypeInfo<T>::size(x);
                count++;
            }
        }

        return contentSize + VarUint4::size(count);
    }
};

template<class... Types> struct AggregateTypeBase 
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { 
		return SignatureGenerator<Types...>::writeTypes(s << "{") << "}";
	}

	template<class S> static inline bool skip(S& s) { 
        return (TypeInfo<Types>::skip(s) && ... && true);
    }

	static constexpr inline bool isConstSize() { 
        return (TypeInfo<Types>::isConstSize() && ... && true);
	}
};

}

#endif /* _RPCTYPEINFO_H_ */

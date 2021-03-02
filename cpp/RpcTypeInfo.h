#ifndef _RPCTYPEINFO_H_
#define _RPCTYPEINFO_H_

#include "RpcCall.h"
#include "RpcVarInt.h"

#include <stddef.h>
#include <stdint.h>

namespace rpc {

/**
 * Serialization rules for boolean values.
 * 
 * A bool value is encoded as a single byte with a value of 0 for false and 1 for true.
 */
template<> struct TypeInfo<bool> { 
	static_assert(sizeof(bool) == 1);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "b"; }
	template<class S> static inline bool write(S& s, const bool &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, bool &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(1); }
	static constexpr inline size_t size(...) { return 1; }
	static constexpr inline bool isConstSize() { return true; }
};

/**
 * Serialization rules for char values.
 * 
 * A char value is encoded as a single byte.
 */
template<> struct TypeInfo<char> { 
	static_assert(sizeof(char) == 1);
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << "i1"; }
	template<class S> static inline bool write(S& s, const char &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, char &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(1); }
	static constexpr inline size_t size(...) { return 1; }
	static constexpr inline bool isConstSize() { return true; }
};

/**
 * Serialization rules for signed char values.
 * 
 * A signed char value is encoded the same way as a char value (are they actually the same?)
 */
template<> struct TypeInfo<signed char>: TypeInfo<char> {};

/**
 * Serialization rules for unsigned char values.
 * 
 * A unsigned char value is encoded as a single byte.
 */
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

/**
 * Serialization rules for signed short values.
 * 
 * A signed short value is encoded as two bytes in little endian (LSB first) order.
 */
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

/**
 * Serialization rules for unsigned short values.
 * 
 * A unsigned short value is encoded as two bytes in little endian (LSB first) order.
 */
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

/**
 * Serialization rules for signed int values.
 * 
 * A signed int value is encoded as four bytes in little endian (LSB first) order.
 */
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

/**
 * Serialization rules for unsigned int values.
 * 
 * A unsigned int value is encoded as four bytes in little endian (LSB first) order.
 */
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

/**
 * Serialization rules for signed long values.
 * 
 * A signed long value is encoded as eight bytes in little endian (LSB first) order.
 */
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

/**
 * Serialization rules for signed long long values.
 * 
 * A signed long value is encoded the same way as a signed long value (are they actually the same?)
 */
template<> struct TypeInfo<signed long long> : TypeInfo<signed long> {};

/**
 * Serialization rules for unsigned long values.
 * 
 * A unsigned long value is encoded as eight bytes in little endian (LSB first) order.
 */
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

/**
 * Serialization rules for unsigned long long values.
 * 
 * A unsigned long value is encoded the same way as a unsigned long value (are they actually the same?)
 */
template<> struct TypeInfo<unsigned long long> : TypeInfo<unsigned long> {};

template<size_t n> class CTStr;

/**
 * Serialization rules for Call objects.
 * 
 * The call object's 32 bit identifier field is encoded using variable length encoding.
 */
template<class... Args> struct TypeInfo<Call<Args...>> 
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return writeSignature<Args...>(s); }
	template<class S> static inline bool write(S& s, const Call<Args...> &v) { return VarUint4::write(s, v.id); }
	template<class S> static inline bool read(S& s, Call<Args...> &v) { return VarUint4::read(s, v.id); }
	template<class S> static inline bool skip(S& s) { return VarUint4::skip(s); }
	static constexpr inline size_t size(const Call<Args...> &v) { return VarUint4::size(v.id); }
	static constexpr inline bool isConstSize() { return false; }
};

/**
 * Common serialization rules for all collection objects.
 * 
 * A collection's length is written first using variable length encoding 
 * then the elements of the collection are written one after the other.
 */
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

/**
 * Common serialization rules for STL-like containers.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class C, class T> struct StlCompatibleCollectionTypeBase: CollectionTypeBase<T>
{ 
    static constexpr inline size_t size(const C& v) 
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

/**
 * Common serialization rules for struct or tuple like (aggregate) object.
 * 
 * An aggregate is encoded simply as its members one after the other.
 */
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

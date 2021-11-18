#ifndef ROLL_CPP_TYPES_PRIMITIVETYPEINFO_H_
#define ROLL_CPP_TYPES_PRIMITIVETYPEINFO_H_

#include "TypeInfo.h"

namespace rpc {

template<> struct TypeInfo<void>
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s; }
};

/**
 * General serialization rules for primitive integer values.
 */
template<class T, char prefix> struct PrimitveIntegerTypeInfoBase
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
	static_assert(prefix == 'i' || prefix == 'u');
	static constexpr const char sgn[3] = {prefix, '0' + (char)sizeof(T), '\0'};

	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return s << sgn; }
	template<class S> static inline bool write(S& s, const T &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, T &v) { return s.read(v); }
	template<class S> static inline bool skip(S& s) { return s.skip(sizeof(T)); }
	static constexpr inline size_t size(...) { return sizeof(T); }
	static constexpr inline bool isConstSize() { return true; }
};

/**
 * Serialization rules for char values.
 *
 * A char value is encoded as a single byte.
 */
template<> struct TypeInfo<char>: PrimitveIntegerTypeInfoBase<char, 'i'> {};

/**
 * Serialization rules for signed char values.
 *
 * A signed char value is encoded the same way as a char value (are they actually the same?)
 */
template<> struct TypeInfo<signed char>: PrimitveIntegerTypeInfoBase<signed char, 'i'> {};

/**
 * Serialization rules for unsigned char values.
 *
 * A unsigned char value is encoded as a single byte.
 */
template<> struct TypeInfo<unsigned char>: PrimitveIntegerTypeInfoBase<unsigned char, 'u'> {};

/**
 * Serialization rules for signed short values.
 *
 * A signed short value is encoded as two bytes in little endian (LSB first) order.
 */
template<> struct TypeInfo<signed short>: PrimitveIntegerTypeInfoBase<signed short, 'i'> {};

/**
 * Serialization rules for unsigned short values.
 *
 * A unsigned short value is encoded as two bytes in little endian (LSB first) order.
 */
template<> struct TypeInfo<unsigned short>: PrimitveIntegerTypeInfoBase<unsigned short, 'u'> {};

/**
 * Serialization rules for signed int values.
 *
 * A signed int value is encoded as four bytes in little endian (LSB first) order.
 */
template<> struct TypeInfo<signed int>: PrimitveIntegerTypeInfoBase<signed int, 'i'> {};

/**
 * Serialization rules for unsigned int values.
 *
 * A unsigned int value is encoded as four bytes in little endian (LSB first) order.
 */
template<> struct TypeInfo<unsigned int>: PrimitveIntegerTypeInfoBase<unsigned int, 'u'> {};

/**
 * Serialization rules for signed long values.
 *
 * A signed long value is encoded as eight bytes in little endian (LSB first) order.
 */
template<> struct TypeInfo<signed long>: PrimitveIntegerTypeInfoBase<signed long, 'i'> {};

/**
 * Serialization rules for unsigned long values.
 *
 * A unsigned long value is encoded as eight bytes in little endian (LSB first) order.
 */
template<> struct TypeInfo<unsigned long>: PrimitveIntegerTypeInfoBase<unsigned long, 'u'> {};

/**
 * Serialization rules for signed long long values.
 *
 * A signed long value is encoded the same way as a signed long value (are they actually the same?)
 */
template<> struct TypeInfo<signed long long>: PrimitveIntegerTypeInfoBase<signed long long, 'i'> {};

/**
 * Serialization rules for unsigned long long values.
 *
 * A unsigned long value is encoded the same way as a unsigned long value (are they actually the same?)
 */
template<> struct TypeInfo<unsigned long long>: PrimitveIntegerTypeInfoBase<unsigned long long, 'u'> {};

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

}

#endif /* ROLL_CPP_TYPES_PRIMITIVETYPEINFO_H_ */

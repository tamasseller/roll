#ifndef COMMON_RPCTYPEINFO_H_
#define COMMON_RPCTYPEINFO_H_

#include "RpcSignatureGenerator.h"
#include "RpcCall.h"

#include <stddef.h>
#include <stdint.h>

template<> struct RpcTypeInfo<bool> { 
	static_assert(sizeof(bool) == 1);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "b"; }
	template<class S> static inline bool write(S& s, const bool &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, bool &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 1; }
};

template<> struct RpcTypeInfo<char> { 
	static_assert(sizeof(char) == 1);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "i1"; }
	template<class S> static inline bool write(S& s, const char &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, char &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 1; }
};

template<> struct RpcTypeInfo<signed char>: RpcTypeInfo<char> {};

template<> struct RpcTypeInfo<unsigned char> 
{ 
	static_assert(sizeof(unsigned char) == 1);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "u1"; }
	template<class S> static inline bool write(S& s, const unsigned char &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned char &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 1; }
};

template<> struct RpcTypeInfo<signed short> 
{
	static_assert(sizeof(signed short) == 2);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "i2"; }
	template<class S> static inline bool write(S& s, const signed short &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, signed short &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 2; }
};

template<> struct RpcTypeInfo<unsigned short> 
{
	static_assert(sizeof(unsigned short) == 2);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "u2"; }
	template<class S> static inline bool write(S& s, const unsigned short &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned short &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 2; }
};

template<> struct RpcTypeInfo<signed int> 
{
	static_assert(sizeof(signed int) == 4);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "i4"; }
	template<class S> static inline bool write(S& s, const signed int &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, signed int &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 4; }
};

template<> struct RpcTypeInfo<unsigned int> 
{ 
	static_assert(sizeof(unsigned int) == 4);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "u4"; }
	template<class S> static inline bool write(S& s, const unsigned int &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned int &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 4; }
};

template<> struct RpcTypeInfo<signed long> 
{
	static_assert(sizeof(signed long) == 8);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "i8"; }
	template<class S> static inline bool write(S& s, const signed long &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, signed long &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 8; }
};

template<> struct RpcTypeInfo<signed long long> : RpcTypeInfo<signed long> {};

template<> struct RpcTypeInfo<unsigned long> 
{ 
	static_assert(sizeof(unsigned long) == 8);
	template<class S> static constexpr inline auto& writeName(S& s) { return s << "u8"; }
	template<class S> static inline bool write(S& s, const unsigned long &v) { return s.write(v); }
	template<class S> static inline bool read(S& s, unsigned long &v) { return s.read(v); }
	static constexpr inline size_t size(...) { return 8; }
};

template<> struct RpcTypeInfo<unsigned long long> : RpcTypeInfo<unsigned long> {};

template<class... Args> struct RpcTypeInfo<RpcCall<Args...>> {
	template<class S> static constexpr inline auto& writeName(S& s) { return writeSignature<Args...>(s); }
	template<class S> static inline bool write(S& s, const RpcCall<Args...> &v) { return s.write(v.id); }
	template<class S> static inline bool read(S& s, RpcCall<Args...> &v) { return s.read(v.id); }
	static constexpr inline size_t size(...) { return 4; }
};

template<class T> struct RpcCollectionTypeBase { 
	template<class S> static constexpr inline auto& writeName(S& s) { 
		return RpcTypeInfo<T>::writeName(s << "[") << "]";
	}
};

template<class... Types> struct RpcAggregateTypeBase { 
	template<class S> static constexpr inline auto& writeName(S& s) { 
		return RpcSignatureGenerator<decltype(s << "{"), Types...>::writeTypes(s << "{") << "}";
	}
};

struct VarUint4
{
	static constexpr inline size_t size(uint32_t c) 
	{
		if(c < 128)
			return 1;
		else if(c < 128 * 128)
			return 2;
		else if(c < 128 * 128 * 128)
			return 3;
		else if(c < 128 * 128 * 128 * 128)
			return 4;

		return 5;
	}

	template<class S> static inline bool write(S& s, uint32_t v) 
	{
		while(v >= 0x80)
		{
			if(!s.write(uint8_t(v | 0x80)))
				return false;

			v >>= 7;
		}

		return s.write(uint8_t(v));
	}

	template<class S> static inline bool read(S& s, uint32_t &v)
	{
		uint8_t nBytes = 0;

		while(true)
		{
			uint8_t d;
			if(!s.read(d))
				return false;

			if(++nBytes < 5)
			{
				v = (v >> 7) | ((uint32_t)d << (32 - 7));

				if(!(d & 0x80))
				{
					v >>= 32 - 7 * nBytes;
					return true;
				}
			}
			else
			{
				v = (v >> 4) | (((uint32_t)d) << 28);
				return true;
			}
		}
	}
};

#endif /* COMMON_RPCTYPEINFO_H_ */

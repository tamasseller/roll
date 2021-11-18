#ifndef ROLL_CPP_TYPES_CSTRINGTYPEINFO_H_
#define ROLL_CPP_TYPES_CSTRINGTYPEINFO_H_

#include "Collection.h"

#include "base/VarInt.h"

namespace rpc
{
/**
 * Serialization rules for a C string array.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<> struct TypeInfo<const char*>: CollectionPlaceholder<char>
{
	static constexpr inline bool isConstSize() { return false; }

	static constexpr inline size_t stringLength(const char* str)
	{
        auto s = str;

		while(*str)
		{
			str++;
		}

		return str - s;
	}

    static constexpr inline size_t size(const char* str)
    {
    	const auto n = stringLength(str);
        return TypeInfo<char>::size('\0') * n + VarUint4::size(n);
    }

	template<class S> static inline bool write(S& s, const char* str)
	{
		auto n = stringLength(str);

		if(!VarUint4::write(s, n))
			return false;

		while(n--)
		{
			if(!TypeInfo<char>::write(s, *str++))
			{
				return false;
			}
		}

		return true;
	}
};

}

#endif /* ROLL_CPP_TYPES_CSTRINGTYPEINFO_H_ */

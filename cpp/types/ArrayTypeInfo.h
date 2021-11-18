#ifndef ROLL_CPP_TYPES_ARRAYTYPEINFO_H_
#define ROLL_CPP_TYPES_ARRAYTYPEINFO_H_

#include "Collection.h"

#include "base/VarInt.h"

namespace rpc {

/**
 * Serialization rules for a simple array.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T, size_t n> struct TypeInfo<T[n]>: CollectionPlaceholder<remove_const_t<T>>
{
	static constexpr inline bool isConstSize() { return TypeInfo<T>::isConstSize(); }

    static constexpr inline size_t size(const T(&v)[n])
    {
        size_t contentSize = 0;

        if constexpr(TypeInfo<T>::isConstSize())
        {
            contentSize = n * TypeInfo<T>::size(v[0]);
        }
        else
        {
            for(const auto &x: v)
                contentSize += TypeInfo<T>::size(x);
        }

        return contentSize + VarUint4::size(n);
    }

	template<class S> static inline bool write(S& s, const T(&v)[n])
	{
		if(VarUint4::write(s, n))
		{
			for(const auto &x: v)
			{
				if(!TypeInfo<T>::write(s, x))
				{
					return false;
				}
			}

			return true;
		}

		return false;
	}

	template<class S> static inline bool read(S& s, T(&v)[n])
	{
		uint32_t count;
		if(VarUint4::read(s, count))
		{
			if(count == n)
			{
				for(auto& x: v)
				{
					if(!TypeInfo<T>::read(s, x))
					{
						return false;
					}
				}

				return true;
			}
		}

		return false;
	}
};

}

#endif /* ROLL_CPP_TYPES_ARRAYTYPEINFO_H_ */

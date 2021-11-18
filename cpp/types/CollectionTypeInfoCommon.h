#ifndef ROLL_CPP_TYPES_COLLECTIONTYPEINFOCOMMON_H_
#define ROLL_CPP_TYPES_COLLECTIONTYPEINFOCOMMON_H_

#include "Collection.h"

#include "base/VarInt.h"

namespace rpc {

/**
 * Common serialization rules for all collection objects.
 *
 * A collection's length is written first using variable length encoding
 * then the elements of the collection are written one after the other.
 */
template<class T> struct CollectionTypeBase: CollectionPlaceholder<T>
{
	template<class S> static inline bool skip(S& s)
    {
        uint32_t count;
        if(!::rpc::VarUint4::read(s, count))
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

        return contentSize + ::rpc::VarUint4::size(count);
    }
};

}

#endif /* ROLL_CPP_TYPES_COLLECTIONTYPEINFOCOMMON_H_ */

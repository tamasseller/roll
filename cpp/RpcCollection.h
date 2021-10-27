#ifndef ROLL_CPP_RPCCOLLECTION_H_
#define ROLL_CPP_RPCCOLLECTION_H_

#include "RpcTypeInfo.h"

namespace rpc {

/**
 * Placeholder for a collection.
 *
 * Supports only signature generator (no read/write).
 * It can also be used as a base-class for serialization rule classes.
 */
template<class T> struct CollectionPlaceholder {
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return TypeInfo<T>::writeName(s << "[") << "]";
	}
};

/**
 * Serialization rules for the collection placeholder.
 *
 * Supports only signature generator (no read/write).
 */
template<class T> struct TypeInfo<CollectionPlaceholder<T>>
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return CollectionPlaceholder<T>::writeName(rpc::move(s));
	}
};

/**
 * Template specialization based global dependency injector for default collection container class.
 */
template<class Element, class Dummy = void> struct CollectionSelector {
	using Type = CollectionPlaceholder<Element>;
};

/**
 * Convenience wrapper for the global collection container dependency injector.
 */
template<class Element> using Many = typename CollectionSelector<Element>::Type;

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

#endif /* ROLL_CPP_RPCCOLLECTION_H_ */

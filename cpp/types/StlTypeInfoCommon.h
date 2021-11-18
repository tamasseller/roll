#ifndef _RPCSTLTYPES_H
#define _RPCSTLTYPES_H

#include "CollectionTypeInfoCommon.h"

#include <iterator>

namespace rpc {

/**
 * Common serialization rules for most STL containers.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class C, class T> struct StlCollection: StlCompatibleCollectionTypeBase<C, T>
{
    template<class S> static inline bool writeLengthAndContent(S& s, uint32_t count, const C& value)
    { 
        if(!VarUint4::write(s, count))
            return false;

        for(const auto &v: value)
            if(!TypeInfo<T>::write(s, v))
                return false;

        return true;
    }

    template<class S> static inline bool write(S& s, const C& v) {
        return writeLengthAndContent(s, v.size(), v);
    }

    template<class S, class A> static inline bool read(S& s, C& v, A&& a)
    { 
        uint32_t count;
        if(!VarUint4::read(s, count))
            return false;

        v.clear();
        auto oit = a(count, v);

        while(count--)
        {
            T v;

            if(!TypeInfo<T>::read(s, v))
                return false;

            *oit++ = std::move(v);
        }

        return true;
    }
};

/**
 * Common serialization rules for associative STL containers.
 * 
 * std::map, std::set and their unordered and multi variants.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class C, class T> struct StlAssociativeCollection: StlCollection<C, T>
{
    template<class S> static inline bool read(S& s, C& v) 
    {
        return StlAssociativeCollection::StlCollection::read(s, v, [](uint32_t count, C& v){
            return std::inserter<C>(v, v.begin());
        });
    }
};

/**
 * Common serialization rules for std::vector and std::string.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class C, class T> struct StlArrayBasedCollection: StlCollection<C, T> 
{
    template<class S> static inline bool read(S& s, C& v) 
    { 
        return StlArrayBasedCollection::StlCollection::read(s, v, [](uint32_t count, C& v){
            v.reserve(count);
            return std::back_insert_iterator<C>(v);
        });
    }
};

/**
 * Common serialization rules for std::list and std::deque.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class C, class T> struct StlListBasedCollection: StlCollection<C, T> 
{
    template<class S> static inline bool read(S& s, C& v) 
    { 
        return StlListBasedCollection::StlCollection::read(s, v, [](uint32_t count, C& v){
            return std::back_insert_iterator<C>(v);
        });
    }
};

}

#endif

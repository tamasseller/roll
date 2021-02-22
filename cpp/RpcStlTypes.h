#ifndef _RPCSTLTYPES_H
#define _RPCSTLTYPES_H

#include "RpcTypeInfo.h"

#include <iterator>

namespace rpc {

template<class T> struct StlCollection: CollectionTypeBase<T>
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

    template<class S, class C> static inline bool writeLengthAndContent(S& s, uint32_t count, const C& v) 
    { 
        if(!VarUint4::write(s, count))
            return false;

        for(const auto &v: v)
            if(!TypeInfo<T>::write(s, v))
                return false;

        return true;
    }

    template<class S, class C> static inline bool write(S& s, const C& v) {
        return writeLengthAndContent(s, v.size(), v);
    }

    template<class S, class C, class A> static inline bool read(S& s, C& v, A&& a)
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

template<class T> struct StlAssociativeCollection: StlCollection<T>
{
    template<class S, class C> static inline bool read(S& s, C& v) 
    {
        return StlAssociativeCollection::StlCollection::read(s, v, [](uint32_t count, C& v){
            return std::inserter<C>(v, v.begin());
        });
    }
};

template<class T> struct StlArrayBasedCollection: StlCollection<T> 
{
    template<class S, class C> static inline bool read(S& s, C& v) 
    { 
        return StlArrayBasedCollection::StlCollection::read(s, v, [](uint32_t count, C& v){
            v.reserve(count);
            return std::back_insert_iterator<C>(v);
        });
    }
};

template<class T> struct StlListBasedCollection: StlCollection<T> 
{
    template<class S, class C> static inline bool read(S& s, C& v) 
    { 
        return StlListBasedCollection::StlCollection::read(s, v, [](uint32_t count, C& v){
            return std::back_insert_iterator<C>(v);
        });
    }
};

}

#endif
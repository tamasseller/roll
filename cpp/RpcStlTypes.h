#ifndef _RPCSTLTYPES_H
#define _RPCSTLTYPES_H

#include "RpcTypeInfo.h"

#include <iterator>

template<class T> struct RpcStlCollection: RpcCollectionTypeBase<T>
{
    template<class C> static constexpr inline size_t size(const C& v) 
    {
        size_t ret = 0;
        uint32_t count = 0;

        for(const auto &x: v)
        {
            ret += RpcTypeInfo<T>::size(x);
            count++;
        }

        return ret + VarUint4::size(count);
    }

    template<class S, class C> static inline bool writeLengthAndContent(S& s, uint32_t count, const C& v) 
    { 
        if(!VarUint4::write(s, count))
            return false;

        for(const auto &v: v)
            if(!RpcTypeInfo<T>::write(s, v))
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

            if(!RpcTypeInfo<T>::read(s, v))
                return false;

            *oit++ = std::move(v);
        }

        return true;
    }
};

template<class T> struct RpcStlAssociativeCollection: RpcStlCollection<T>
{
    template<class S, class C> static inline bool read(S& s, C& v) 
    {
        return RpcStlAssociativeCollection::RpcStlCollection::read(s, v, [](uint32_t count, C& v){
            return std::inserter<C>(v, v.begin());
        });
    }
};

template<class T> struct RpcStlArrayBasedCollection: RpcStlCollection<T> 
{
    template<class S, class C> static inline bool read(S& s, C& v) 
    { 
        return RpcStlArrayBasedCollection::RpcStlCollection::read(s, v, [](uint32_t count, C& v){
            v.reserve(count);
            return std::back_insert_iterator<C>(v);
        });
    }
};

template<class T> struct RpcStlListBasedCollection: RpcStlCollection<T> 
{
    template<class S, class C> static inline bool read(S& s, C& v) 
    { 
        return RpcStlListBasedCollection::RpcStlCollection::read(s, v, [](uint32_t count, C& v){
            return std::back_insert_iterator<C>(v);
        });
    }
};

#endif
#ifndef _RPCSTLLIST_H_
#define _RPCSTLLIST_H_

#include "RpcStlTypes.h"

#include <list>
#include <deque>
#include <forward_list>

namespace rpc {

template<class T> struct TypeInfo<std::list<T>>: StlListBasedCollection<T> {};
template<class T> struct TypeInfo<std::deque<T>>: StlListBasedCollection<T> {};

template<class T> struct TypeInfo<std::forward_list<T>>: StlCollection<T> 
{
    template<class S> static inline bool write(S& s, const std::forward_list<T>& v) {
        return TypeInfo::StlCollection::writeLengthAndContent(s, std::distance(v.begin(), v.end()), v);
    }

    template<class S> static inline bool read(S& s, std::forward_list<T>& v) 
    { 
        bool ok = TypeInfo::StlCollection::read(s, v, [](uint32_t count, std::forward_list<T>& v){
            return std::front_insert_iterator<std::forward_list<T>>(v);
        });

        if(ok)
        {
            v.reverse();
            return true;
        }

        return false;
    }

    template<class C> static constexpr inline size_t size(const C& v) 
    {       
        size_t contentSize = 0;
        uint32_t count = 0;

        for(const auto &x: v)
        {
            contentSize += TypeInfo<T>::size(x);
            count++;
        }

        return contentSize + VarUint4::size(count);
    }
};

}

#endif /* _RPCSTLLIST_H_ */

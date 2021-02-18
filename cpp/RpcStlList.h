#ifndef _RPCSTLLIST_H_
#define _RPCSTLLIST_H_

#include "RpcStlTypes.h"

#include <list>
template<class T> struct RpcTypeInfo<std::list<T>>: RpcStlListBasedCollection<T> {};

#include <deque>
template<class T> struct RpcTypeInfo<std::deque<T>>: RpcStlListBasedCollection<T> {};

#include <forward_list>
template<class T> struct RpcTypeInfo<std::forward_list<T>>: RpcStlCollection<T> 
{
    template<class S> static inline bool write(S& s, const std::forward_list<T>& v) {
        return RpcTypeInfo::RpcStlCollection::writeLengthAndContent(s, std::distance(v.begin(), v.end()), v);
    }

    template<class S> static inline bool read(S& s, std::forward_list<T>& v) 
    { 
        bool ok = RpcTypeInfo::RpcStlCollection::read(s, v, [](uint32_t count, std::forward_list<T>& v){
            return std::front_insert_iterator<std::forward_list<T>>(v);
        });

        if(ok)
        {
            v.reverse();
            return true;
        }

        return false;
    }
};

#endif /* _RPCSTLLIST_H_ */

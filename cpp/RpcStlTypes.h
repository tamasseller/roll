#ifndef _RPC_STL_TYPES_H
#define _RPC_STL_TYPES_H

#include "RpcTypeInfo.h"

#include <string>

#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

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

template<class T> struct RpcTypeInfo<std::vector<T>>: RpcStlArrayBasedCollection<T> {};
template<> struct RpcTypeInfo<std::string>: RpcStlArrayBasedCollection<char> {};

template<class T> struct RpcTypeInfo<std::list<T>>: RpcStlListBasedCollection<T> {};
template<class T> struct RpcTypeInfo<std::deque<T>>: RpcStlListBasedCollection<T> {};
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

template<class T> struct RpcTypeInfo<std::set<T>>: RpcStlAssociativeCollection<T> {};
template<class T> struct RpcTypeInfo<std::multiset<T>>: RpcStlAssociativeCollection<T> {};
template<class T> struct RpcTypeInfo<std::unordered_set<T>>: RpcStlAssociativeCollection<T> {};
template<class T> struct RpcTypeInfo<std::unordered_multiset<T>>: RpcStlAssociativeCollection<T> {};

template<class K, class V> struct RpcTypeInfo<std::map<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct RpcTypeInfo<std::multimap<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct RpcTypeInfo<std::unordered_map<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct RpcTypeInfo<std::unordered_multimap<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};


template<class... Types> struct RpcTypeInfo<std::tuple<Types...>>: RpcAggregateTypeBase<Types...>
{
   	static constexpr inline size_t size(const std::tuple<Types...>& v)  {
        return std::apply([] (auto&&... x) { return (RpcTypeInfo<Types>::size(x) + ... + 0); }, v);
    }

	template<class S> static inline bool write(S& s, const std::tuple<Types...>& v) { 
        return std::apply([&s] (auto&&... x) { return (RpcTypeInfo<Types>::write(s, x) && ... && true); }, v); 
    }

	template<class S> static inline bool read(S& s, std::tuple<Types...>& v) { 
        return std::apply([&s] (auto&&... x) { return (RpcTypeInfo<Types>::read(s, x) && ... && true); }, v);
    }
};

template<class T1, class T2> struct RpcTypeInfo<std::pair<T1, T2>>: RpcAggregateTypeBase<T1, T2>
{
	static constexpr inline size_t size(const std::pair<T1, T2>& v)  {
        return RpcTypeInfo<T1>::size(v.first) + RpcTypeInfo<T2>::size(v.second); 
    }

	template<class S> static inline bool write(S& s, const std::pair<T1, T2>& v) { 
        return RpcTypeInfo<T1>::write(s, v.first) && RpcTypeInfo<T2>::write(s, v.second); 
    }

	template<class S> static inline bool read(S& s, std::pair<T1, T2>& v) { 
        return RpcTypeInfo<T1>::read(s, v.first) && RpcTypeInfo<T2>::read(s, v.second); 
    }
};

#endif
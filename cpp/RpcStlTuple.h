#ifndef _RPCSTLTUPLE_H_
#define _RPCSTLTUPLE_H_

#include "RpcStlTypes.h"

#include <tuple>
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

#include <utility>
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

#endif /* _RPCSTLTUPLE_H_ */

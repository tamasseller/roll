#ifndef _RPCSTLTUPLE_H_
#define _RPCSTLTUPLE_H_

#include "RpcStlTypes.h"

#include <tuple>
#include <utility>

namespace rpc {

template<class... Types> struct TypeInfo<std::tuple<Types...>>: AggregateTypeBase<Types...>
{
   	static constexpr inline size_t size(const std::tuple<Types...>& v)  {
        return std::apply([] (auto&&... x) { return (TypeInfo<Types>::size(x) + ... + 0); }, v);
    }

	template<class S> static inline bool write(S& s, const std::tuple<Types...>& v) { 
        return std::apply([&s] (auto&&... x) { return (TypeInfo<Types>::write(s, x) && ... && true); }, v); 
    }

	template<class S> static inline bool read(S& s, std::tuple<Types...>& v) { 
        return std::apply([&s] (auto&&... x) { return (TypeInfo<Types>::read(s, x) && ... && true); }, v);
    }
};

template<class T1, class T2> struct TypeInfo<std::pair<T1, T2>>: AggregateTypeBase<T1, T2>
{
	static constexpr inline size_t size(const std::pair<T1, T2>& v)  {
        return TypeInfo<T1>::size(v.first) + TypeInfo<T2>::size(v.second); 
    }

	template<class S> static inline bool write(S& s, const std::pair<T1, T2>& v) { 
        return TypeInfo<T1>::write(s, v.first) && TypeInfo<T2>::write(s, v.second); 
    }

	template<class S> static inline bool read(S& s, std::pair<T1, T2>& v) { 
        return TypeInfo<T1>::read(s, v.first) && TypeInfo<T2>::read(s, v.second); 
    }
};

}

#endif /* _RPCSTLTUPLE_H_ */

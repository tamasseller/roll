#ifndef _RPCSTLSET_H_
#define _RPCSTLSET_H_

#include "RpcStlTypes.h"

#include <set>
template<class T> struct RpcTypeInfo<std::set<T>>: RpcStlAssociativeCollection<T> {};
template<class T> struct RpcTypeInfo<std::multiset<T>>: RpcStlAssociativeCollection<T> {};

#include <unordered_set>
template<class T> struct RpcTypeInfo<std::unordered_set<T>>: RpcStlAssociativeCollection<T> {};
template<class T> struct RpcTypeInfo<std::unordered_multiset<T>>: RpcStlAssociativeCollection<T> {};

#endif /* _RPCSTLSET_H_ */

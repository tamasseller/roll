#ifndef _RPCSTLSET_H_
#define _RPCSTLSET_H_

#include "RpcStlTypes.h"

#include <set>
#include <unordered_set>

namespace rpc {

template<class T> struct TypeInfo<std::set<T>>: StlAssociativeCollection<T> {};
template<class T> struct TypeInfo<std::multiset<T>>: StlAssociativeCollection<T> {};

template<class T> struct TypeInfo<std::unordered_set<T>>: StlAssociativeCollection<T> {};
template<class T> struct TypeInfo<std::unordered_multiset<T>>: StlAssociativeCollection<T> {};

}

#endif /* _RPCSTLSET_H_ */

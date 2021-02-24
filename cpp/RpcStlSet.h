#ifndef _RPCSTLSET_H_
#define _RPCSTLSET_H_

#include "RpcStlTypes.h"

#include <set>
#include <unordered_set>

namespace rpc {

/**
 * Serialization rules for std::set.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::set<T>>: StlAssociativeCollection<std::set<T>, T> {};

/**
 * Serialization rules for std::multiset.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::multiset<T>>: StlAssociativeCollection<std::multiset<T>, T> {};

/**
 * Serialization rules for std::unordered_set.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::unordered_set<T>>: StlAssociativeCollection<std::unordered_set<T>, T> {};

/**
 * Serialization rules for std::unordered_multiset.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::unordered_multiset<T>>: StlAssociativeCollection<std::unordered_multiset<T>, T> {};

}

#endif /* _RPCSTLSET_H_ */

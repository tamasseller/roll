#ifndef _RPCSTLMAP_H_
#define _RPCSTLMAP_H_

#include "RpcStlTypes.h"

#include <map>
#include <unordered_map>

namespace rpc {

/**
 * Serialization rules for std::map.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::map<K, V>>: StlAssociativeCollection<std::map<K, V>, std::pair<K, V>> {};

/**
 * Serialization rules for std::multimap.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::multimap<K, V>>: StlAssociativeCollection<std::multimap<K, V>, std::pair<K, V>> {};

/**
 * Serialization rules for std::unordered_map.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::unordered_map<K, V>>: StlAssociativeCollection<std::unordered_map<K, V>, std::pair<K, V>> {};

/**
 * Serialization rules for std::unordered_multimap.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::unordered_multimap<K, V>>: StlAssociativeCollection<std::unordered_multimap<K, V>, std::pair<K, V>> {};
    
}

#endif /* _RPCSTLMAP_H_ */

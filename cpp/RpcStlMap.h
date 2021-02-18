#ifndef _RPCSTLMAP_H_
#define _RPCSTLMAP_H_

#include "RpcStlTypes.h"

#include <map>
template<class K, class V> struct RpcTypeInfo<std::map<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct RpcTypeInfo<std::multimap<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};

#include <unordered_map>
template<class K, class V> struct RpcTypeInfo<std::unordered_map<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct RpcTypeInfo<std::unordered_multimap<K, V>>: RpcStlAssociativeCollection<std::pair<K, V>> {};


#endif /* _RPCSTLMAP_H_ */

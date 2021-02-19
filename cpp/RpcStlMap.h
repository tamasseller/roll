#ifndef _RPCSTLMAP_H_
#define _RPCSTLMAP_H_

#include "RpcStlTypes.h"

#include <map>
#include <unordered_map>

namespace rpc {

template<class K, class V> struct TypeInfo<std::map<K, V>>: StlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct TypeInfo<std::multimap<K, V>>: StlAssociativeCollection<std::pair<K, V>> {};

template<class K, class V> struct TypeInfo<std::unordered_map<K, V>>: StlAssociativeCollection<std::pair<K, V>> {};
template<class K, class V> struct TypeInfo<std::unordered_multimap<K, V>>: StlAssociativeCollection<std::pair<K, V>> {};
    
}

#endif /* _RPCSTLMAP_H_ */

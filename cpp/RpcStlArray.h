#ifndef _RPCSTLARRAY_H_
#define _RPCSTLARRAY_H_

#include "RpcStlTypes.h"

#include <string>
#include <vector>

namespace rpc {

/**
 * Serialization rules for std::string.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<> struct TypeInfo<std::string>: StlArrayBasedCollection<std::string, char> {};

/**
 * Serialization rules for std::vector.
 * 
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::vector<T>>: StlArrayBasedCollection<std::vector<T>, T> {};

}

#endif /* _RPCSTLARRAY_H_ */

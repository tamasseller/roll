#ifndef _RPCSTLARRAY_H_
#define _RPCSTLARRAY_H_

#include "RpcStlTypes.h"

#include <string>
template<> struct RpcTypeInfo<std::string>: RpcStlArrayBasedCollection<char> {};

#include <vector>
template<class T> struct RpcTypeInfo<std::vector<T>>: RpcStlArrayBasedCollection<T> {};


#endif /* _RPCSTLARRAY_H_ */

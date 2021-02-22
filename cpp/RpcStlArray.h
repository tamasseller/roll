#ifndef _RPCSTLARRAY_H_
#define _RPCSTLARRAY_H_

#include "RpcStlTypes.h"

#include <string>
#include <vector>

namespace rpc {

template<> struct TypeInfo<std::string>: StlArrayBasedCollection<std::string, char> {};
template<class T> struct TypeInfo<std::vector<T>>: StlArrayBasedCollection<std::vector<T>, T> {};

}

#endif /* _RPCSTLARRAY_H_ */

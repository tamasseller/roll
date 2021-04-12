#ifndef _RPCSTLSIMPLEFACADE_H_
#define _RPCSTLSIMPLEFACADE_H_

#include "RpcEndpoint.h"
#include "RpcStlAdapters.h"

#include <string.h>

namespace rpc {

template<class IoEngine> using StlSimpleFacade
		= Endpoint<detail::StlAutoPointer, detail::HashMapRegistry, detail::HashMapBasedNameDictionary, IoEngine>;

}

#endif /* _RPCSTLSIMPLEFACADE_H_ */

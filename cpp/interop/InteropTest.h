#ifndef _INTEROPTEST_H_
#define _INTEROPTEST_H_

#include "RpcStlFacade.h"
#include "RpcFdStreamAdapter.h"

#include <memory>
#include <thread>

using Rpc = rpc::StlFacade<rpc::FdStreamAdapter>;

std::thread runInteropTests(std::shared_ptr<Rpc>);

#endif /* _INTEROPTEST_H_ */

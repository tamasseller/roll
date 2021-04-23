#ifndef _INTEROPTEST_H_
#define _INTEROPTEST_H_

#include "RpcStlFacade.h"
#include "RpcFdStreamAdapter.h"

#include <memory>
#include <thread>

struct Rpc: rpc::StlFacade<Rpc, rpc::PreallocatedMemoryBufferStreamWriterFactory>, rpc::FdStreamAdapter
{
    using FdStreamAdapter::FdStreamAdapter;
};

std::thread runInteropTests(std::shared_ptr<Rpc>);

#endif /* _INTEROPTEST_H_ */

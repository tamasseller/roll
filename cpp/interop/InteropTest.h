#ifndef _INTEROPTEST_H_
#define _INTEROPTEST_H_

#include "RpcClient.h"
#include "RpcFdStreamAdapter.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"

#include "Contract.gen.h"

#include <thread>
#include <memory>

namespace syms
{
    static constexpr auto nope = rpc::symbol<>("nope"_ctstr);

    using Close = rpc::Call<>;
    using Read = rpc::Call<uint32_t, rpc::Call<std::list<uint16_t>>>;
    using Methods = std::tuple<Close, Read>;
    static constexpr auto open = rpc::symbol<uint8_t, uint8_t, rpc::Call<Methods>>("open"_ctstr);
}

using Client = InteropTestClientProxy<rpc::FdStreamAdapter>;

//template<class Child, class Adapter>
//struct InteropTestServerProxy: rpc::StlEndpoint<Adapter>
//{
//    using Endpoint = typename InteropTestServerProxy::StlEndpoint::Endpoint;
//
//    template<class... Args>
//    InteropTestServerProxy(Args&&... args):
//		InteropTestServerProxy::StlEndpoint(std::forward<Args>(args)...)
//	{
//    	{
//			static_assert(rpc::nArgs<&Child::unlock> == 1, "Public method unlock must take 1 argument");
//			static_assert(rpc::isCompatible<rpc::Arg<0, &Child::unlock>, bool>(), "Public method unlock must take argument #1 compatible with 'str'");
//
//			auto err = this->provide(InteropTestSymbols::symUnlock, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle h, rpc::Arg<0, &Child::unlock> doIt)
//			{
//				self->unlock(std::move(doIt));
//			});
//
//			if(err)
//			{
//				rpc::fail(std::string("Registering public method unlock resulted in error ") + err);
//			}
//    	}
//	}
//};

struct Service: InteropTestServerProxy<Service, rpc::FdStreamAdapter>
{
	std::mutex m;
	std::condition_variable cv;
	volatile bool locked = true;
	volatile int makeCnt = 0, delCnt = 0;

	using InteropTestServerProxy::InteropTestServerProxy;
	void unlock(bool doIt);

	void wait();

	std::string echo(const std::string&);
};

std::thread runInteropTests(std::shared_ptr<Client>);
std::thread runInteropListener(std::shared_ptr<Service>);

#endif /* _INTEROPTEST_H_ */

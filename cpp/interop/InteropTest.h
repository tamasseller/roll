#ifndef _INTEROPTEST_H_
#define _INTEROPTEST_H_

#include "RpcFdStreamAdapter.h"
#include "RpcClient.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"

#include "Contract.gen.h"

#include <thread>
#include <memory>

namespace syms
{
    static constexpr auto echo = rpc::symbol<std::string, rpc::Call<std::string>>("echo"_ctstr);
    static constexpr auto nope = rpc::symbol<>("nope"_ctstr);
    static constexpr auto unlock = rpc::symbol<bool>("unlock"_ctstr);
    using Close = rpc::Call<>;
    using Read = rpc::Call<uint32_t, rpc::Call<std::list<uint16_t>>>;
    using Methods = std::tuple<Close, Read>;
    static constexpr auto open = rpc::symbol<uint8_t, uint8_t, rpc::Call<Methods>>("open"_ctstr);
}

template<class Child>
class RpcInteropTest: public rpc::ClientBase<Child>
{
    rpc::OnDemand<decltype(syms::unlock)> unlockCall = syms::unlock;
    rpc::OnDemand<decltype(syms::echo)> echoCall = syms::echo;

public:
    using RpcInteropTest::ClientBase::ClientBase;

	template<class A0>
    inline auto unlock(A0&& doIt) {
		static_assert(rpc::isCompatible<A0, bool>(), "First argument of echo must be of type 'bool'");
		return this->callAction(unlockCall, std::forward<A0>(doIt));
    }

    template<class A0, class C>
    inline auto echo(A0&& v, C&& c)
    {
    	static_assert(rpc::isCompatible<A0, rpc::CollectionPlaceholder<int8_t>>(), "First argument of echo must be of type 'str'");
    	static_assert(rpc::isCompatible<rpc::Arg<0, C>, rpc::CollectionPlaceholder<int8_t>>(), "Callback for echo must take a first argument compatible with 'str'");
    	return this->callWithCallback(echoCall, std::move(c), std::forward<A0>(v));
    }

    template<class Ret, class A0>
    inline auto echo(A0&& v)
    {
    	static_assert(rpc::isCompatible<A0, rpc::CollectionPlaceholder<int8_t>>(), "First argument of echo must be of type 'str'");
    	static_assert(rpc::isCompatible<Ret, rpc::CollectionPlaceholder<int8_t>>(), "Echo can only return a type compatible with 'str'");
    	return this->template callWithPromise<Ret>(echoCall, std::forward<A0>(v));
    }
};

struct Rpc: InteropTestClientProxy<rpc::FdStreamAdapter> {
    using Rpc::InteropTestClientProxy::InteropTestClientProxy;
};

std::thread runInteropTests(std::shared_ptr<Rpc>);

#endif /* _INTEROPTEST_H_ */

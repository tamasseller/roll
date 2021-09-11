#ifndef _INTEROPTEST_H_
#define _INTEROPTEST_H_

#include "RpcFdStreamAdapter.h"
#include "RpcClient.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"

#include <thread>
#include <memory>
#include <future>

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

template<class> struct FirstArgument;
template<class C, class A> struct FirstArgument<void(C::*)(A)> { using T = A; };
template<class C, class A> struct FirstArgument<void(C::*)(A) const> { using T = A; };

template<class Child>
class RpcInteropTest: public rpc::ClientBase<Child>
{
    using RpcInteropTest::ClientBase::ClientBase;

    rpc::OnDemand<decltype(syms::unlock)> unlockCall = syms::unlock;
    rpc::OnDemand<decltype(syms::echo)> echoCall = syms::echo;

public:
	using Endpoint = typename RpcInteropTest::ClientBase::Endpoint;

	template<class... Args>
    inline void unlock(Args... args) {
    	this->unlockCall.call(*this, std::forward<Args>(args)...);
    }

    template<class C, class A0>
    inline void echo(A0&& v, C&& c)
    {
    	auto id = this->install([c{std::move(c)}](Endpoint& rpc, rpc::MethodHandle h, typename FirstArgument<decltype(&C::operator())>::T arg)
		{
    		c(rpc::move(arg));
    		rpc.uninstall(h);
    	});

    	this->echoCall.call(*this, std::forward<A0>(v), id);
    }

    template<class T, class A0>
    inline std::future<T> echo(A0&& v)
    {
    	std::promise<T> p;
    	auto f = p.get_future();

    	auto id = this->install([p{std::move(p)}](Endpoint& rpc, rpc::MethodHandle h, T arg) mutable
		{
    		p.set_value(arg);
    		rpc.uninstall(h);
    	});

    	this->echoCall.call(*this, std::forward<A0>(v), id);
    	return f;
    }
};

struct Rpc: RpcInteropTest<rpc::FdStreamAdapter> {
    using Rpc::RpcInteropTest::RpcInteropTest;
};

std::thread runInteropTests(std::shared_ptr<Rpc>);

#endif /* _INTEROPTEST_H_ */

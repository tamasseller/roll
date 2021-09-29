#ifndef RPC_CPP_RPCSERVICE_H_
#define RPC_CPP_RPCSERVICE_H_

#include "RpcUtility.h"
#include "RpcEndpoint.h"

namespace rpc {

template<class EpArg>
class ServiceBase: public EpArg
{
public:
    using Endpoint = typename EpArg::Endpoint;

protected:
	template<auto &sym, class Child, auto member, class... Args>
	void provideAction()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint&, rpc::MethodHandle, Args... args)
		{
			(self->*member)(rpc::forward<Args>(args)...);
		});

		if(err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", err);
		}
	}

	template<auto &sym, class Child, auto member, class Ret, class... Args>
	void provideFunction()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle, Args... args, rpc::Call<Ret> cb)
		{
			if(auto err = ep.call(cb, (self->*member)(rpc::forward<Args>(args)...)))
			{
				rpc::fail("Calling callback of '", (const char*)sym, "': ", err);
			}
		});

		if(err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", err);
		}
	}

	template<auto &sym, class Child, auto member, class Exports, class Accept, class... Args>
	void provideCtor()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle, Args... args, Exports exports, Accept accept)
		{
			auto obj = (self->*member)(rpc::forward<Args>(args)...);

			if(auto err = ep.call(accept, obj->exportLocal(ep, obj)))
			{
				rpc::fail("Calling accept for '", (const char*)sym, "': ", err);
			}

			obj->importRemote(exports);
		});

		if(err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", err);
		}
	}

	template<auto &sym, class Child, auto member, class Exports, class Accept, class... Args>
	void provideCtorWithRetval()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle, Args... args, Exports exports, Accept accept)
		{
			auto pair = (self->*member)(rpc::forward<Args>(args)...);
			auto ret = rpc::move(pair.first);
			auto obj = rpc::move(pair.second);

			if(auto err = ep.call(accept, rpc::move(ret), obj->exportLocal(ep, obj)))
			{
				rpc::fail("Calling accept for '", (const char*)sym, "': ", err);
			}

			obj->importRemote(exports);
		});

		if(err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", err);
		}
	}

public:
    using Endpoint::call;
    using Endpoint::install;
    using Endpoint::uninstall;
    using Endpoint::lookup;
    using Endpoint::provide;
    using Endpoint::discard;
    using Endpoint::process;

    using EpArg::EpArg;
};

}
#endif /* RPC_CPP_RPCSERVICE_H_ */

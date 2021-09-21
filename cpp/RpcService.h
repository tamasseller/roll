#ifndef RPC_CPP_RPCSERVICE_H_
#define RPC_CPP_RPCSERVICE_H_

#include "RpcUtility.h"
#include "RpcEndpoint.h"
#include "RpcStlAdapters.h"

namespace rpc {

template<class Io>
class ServiceBase: StlEndpoint<Io>
{
public:
    using Endpoint = typename ServiceBase::StlEndpoint::Endpoint;

protected:
	template<auto &sym, class Child, auto member, class... Args>
	void provideAction()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint&, rpc::MethodHandle, Args... args)
		{
			(self->*member)(std::forward<Args>(args)...);
		});

		if(err)
		{
			rpc::fail(std::string("Registering public method '") + (const char*)sym + "' resulted in error " + err);
		}
	}

	template<auto &sym, class Child, auto member, class Ret, class... Args>
	void provideFunction()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle, Args... args, rpc::Call<Ret> cb)
		{
			if(auto err = ep.call(cb, (self->*member)(std::forward<Args>(args)...)))
			{
				rpc::fail(std::string("Calling callback of public method '") + (const char*)sym + "' resulted in error " + err);
			}
		});

		if(err)
		{
			rpc::fail(std::string("Registering public method '") + (const char*)sym + "' resulted in error " + err);
		}
	}

	template<auto &sym, class Child, auto member, class Exports, class Accept, class... Args>
	void provideCtor()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle, Args... args, Exports exports, Accept accept)
		{
			auto obj = (self->*member)(std::forward<Args>(args)...);

			if(auto err = ep.call(accept, obj->exportLocal(ep, obj)))
			{
				rpc::fail(std::string("Calling accept callback for session constructor '") + (const char*)sym + "' resulted in error " + err);
			}

			obj->importRemote(exports);
		});

		if(err)
		{
			rpc::fail(std::string("Registering public method '") + (const char*)sym + "' resulted in error " + err);
		}
	}

	template<auto &sym, class Child, auto member, class Exports, class Accept, class... Args>
	void provideCtorWithRetval()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](Endpoint& ep, rpc::MethodHandle, Args... args, Exports exports, Accept accept)
		{
			auto pair = (self->*member)(std::forward<Args>(args)...);
			auto ret = std::move(pair.first);
			auto obj = std::move(pair.second);

			if(auto err = ep.call(accept, std::move(ret), obj->exportLocal(ep, obj)))
			{
				rpc::fail(std::string("Calling accept callback for session constructor '") + (const char*)sym + "' resulted in error " + err);
			}

			obj->importRemote(exports);
		});

		if(err)
		{
			rpc::fail(std::string("Registering public method '") + (const char*)sym + "' resulted in error " + err);
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

    using ServiceBase::StlEndpoint::StlEndpoint;
};

}
#endif /* RPC_CPP_RPCSERVICE_H_ */

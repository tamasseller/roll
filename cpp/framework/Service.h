#ifndef RPC_CPP_RPCSERVICE_H_
#define RPC_CPP_RPCSERVICE_H_

#include "common/Utility.h"

#include "base/RpcEndpoint.h"

#include "Tracker.h"

namespace rpc {

template<class Endpoint>
class ServiceBase: public Endpoint, Tracker
{
	template<class, class, size_t> friend class SessionBase;

protected:
	template<auto &sym, class Child, auto member, class... Args>
	void provideAction()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](ServiceBase&, rpc::MethodHandle, Args... args)
		{
			(self->*member)(rpc::forward<Args>(args)...);
		});

		if(!!err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
		}
	}

	template<auto &sym, class Child, auto member, class Ret, class... Args>
	void provideFunction()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](ServiceBase& ep, rpc::MethodHandle, Args... args, rpc::Call<Ret> cb)
		{
			if(auto err = ep.call(cb, (self->*member)(rpc::forward<Args>(args)...)); !!err)
			{
				rpc::fail("Calling callback of '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
			}
		});

		if(!!err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
		}
	}

	template<auto &sym, class Child, auto member, class Exports, class Accept, class... Args>
	void provideCtor()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](ServiceBase& ep, rpc::MethodHandle, Args... args, Exports exports, Accept accept)
		{
			auto obj = (self->*member)(rpc::forward<Args>(args)...);

			if(auto err = ep.call(accept, obj->exportLocal(ep, obj)); !!err)
			{
				rpc::fail("Calling accept for '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
			}

			obj->importRemote(exports);
		});

		if(!!err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
		}
	}

	template<auto &sym, class Child, auto member, class Exports, class Accept, class... Args>
	void provideCtorWithRetval()
	{
		auto err = this->provide(sym, [self{static_cast<Child*>(this)}](ServiceBase& ep, rpc::MethodHandle, Args... args, Exports exports, Accept accept)
		{
			auto pair = (self->*member)(rpc::forward<Args>(args)...);
			auto ret = rpc::move(pair.first);
			auto obj = rpc::move(pair.second);

			if(auto err = ep.call(accept, rpc::move(ret), obj->exportLocal(ep, obj)); !!err)
			{
				rpc::fail("Calling accept for '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
			}

			obj->importRemote(exports);
		});

		if(!!err)
		{
			rpc::fail("Registering '", (const char*)sym, "': ", getErrorString(err)); /* GCOV_EXCL_LINE */
		}
	}

public:
    using Endpoint::Endpoint;

    inline void connectionClosed() {
    	this->Tracker::notifySubobjects(*this);
	}
};

}
#endif /* RPC_CPP_RPCSERVICE_H_ */

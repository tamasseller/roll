#ifndef RPC_CPP_INTEROP_RPCCLIENT_H_
#define RPC_CPP_INTEROP_RPCCLIENT_H_

#include "RpcUtility.h"
#include "RpcEndpoint.h"
#include "RpcStlAdapters.h"

#include <mutex>
#include <future>
#include <condition_variable>

namespace rpc {

template<class Io>
class ClientBase: StlEndpoint<Io>
{
    friend typename ClientBase::Endpoint;

    volatile bool locked;
    std::mutex m;
    std::condition_variable cv;

    inline void waitLookup()
    {
    	if(locked)
    	{
    		std::unique_lock<std::mutex> l(m);

    		while(locked)
    		{
    			cv.wait(l);
    		}
    	}
    }

    inline void suspendCalls()
    {
   		std::lock_guard l(m);
   		locked = true;
    }

    inline void resumeCalls()
	{
		std::lock_guard l(m);
		locked = false;
		cv.notify_all();
	}

protected:
	template<class Sym> class OnDemand
	{
		volatile bool lookupDone = false;
		typename Sym::CallType callId;
		const Sym& sym;

		template<class Rpc, class = void> struct Lock{
			inline Lock(Rpc&) {}
		};

		template<class Rpc>
		struct Lock<Rpc, decltype(Rpc::lock)>
		{
			Rpc& rpc;

			Lock(Rpc& rpc): rpc(rpc)
			{
				rpc.lock();
			}

			~Lock()
			{
				rpc.unlock();
			}
		};

	public:
		constexpr OnDemand(const Sym& sym): sym(sym) {}

		template<class Rpc, class... Args>
		inline auto call(Rpc& rpc, Args&&... args)
		{
			if(lookupDone)
			{
				rpc.waitLookup();
				rpc.call(this->callId, rpc::forward<Args>(args)...);
			}
			else
			{
				rpc.suspendCalls();
				rpc.lookup(sym, [=, &rpc](auto&, bool done, auto result) mutable
				{
					if(done)
					{
						rpc.call(result, rpc::forward<Args>(args)...);
						rpc.resumeCalls();

						Lock<Rpc> _(rpc);
						this->callId = result;
						this->lookupDone = true;
					}
					else
					{
						fail(std::string("failed to look up symbol '") + (const char*)sym + "'");
					}
				});
			}
		}
	};

    template<class Call, class... Args>
    inline void callAction(Call& call, Args&&... args) {
    	call.call(*this, std::forward<Args>(args)...);
    }

    template<class Call, class Callback, class... Args>
    inline void callWithCallback(Call& call, Callback&& cb, Args&&... args)
    {
    	auto id = this->install([cb{std::move(cb)}](Endpoint& rpc, rpc::MethodHandle h, rpc::Arg<0, &Callback::operator()> arg)
		{
    		cb(rpc::move(arg));
    		rpc.uninstall(h);
    	});

    	call.call(*this, std::forward<Args>(args)..., id);
    }

    template<class Ret, class Call, class... Args>
    inline std::future<Ret> callWithPromise(Call& call, Args&&... args)
    {
    	std::promise<Ret> p;
    	auto f = p.get_future();

    	auto id = this->install([p{std::move(p)}](Endpoint& rpc, rpc::MethodHandle h, Ret arg) mutable
		{
    		p.set_value(arg);
    		rpc.uninstall(h);
    	});

    	call.call(*this, std::forward<Args>(args)..., id);
    	return f;
    }

    template<class Call, class Obj, class Callback, class... Args>
    inline void createWithCallback(Call& call, Obj obj, Callback&& cb, Args&&... args)
    {
    	auto id = this->install([cb{std::move(cb)}, obj](Endpoint& rpc, rpc::MethodHandle h, rpc::Arg<0, &rpc::remove_cref_t<decltype(*obj)>::importRemote> import)
		{
    		obj->importRemote(import);
    		cb();
    		rpc.uninstall(h);
    	});

    	call.call(*this, std::forward<Args>(args)..., obj->exportLocal((Endpoint&)*this, obj), id);
    }

    template<class Call, class Obj, class Callback, class... Args>
    inline void createWithCallbackRetval(Call& call, Obj obj, Callback&& cb, Args&&... args)
    {
    	auto id = this->install([cb{std::move(cb)}, obj](Endpoint& rpc, rpc::MethodHandle h, rpc::Arg<0, &Callback::operator()> arg, rpc::Arg<0, &rpc::remove_cref_t<decltype(*obj)>::importRemote> import)
		{
    		obj->importRemote(import);
    		cb(rpc::move(arg));
    		rpc.uninstall(h);
    	});

    	call.call(*this, std::forward<Args>(args)..., obj->exportLocal((Endpoint&)*this, obj), id);
    }

    template<class Call, class Obj, class... Args>
    inline std::future<void> createWithPromise(Call& call, Obj obj, Args&&... args)
    {
    	std::promise<void> p;
    	auto f = p.get_future();

    	auto id = this->install([p{std::move(p)}, obj](Endpoint& rpc, rpc::MethodHandle h, rpc::Arg<0, &rpc::remove_cref_t<decltype(*obj)>::importRemote> import) mutable
		{
    		obj->importRemote(import);
    		p.set_value();
    		rpc.uninstall(h);
    	});

    	call.call(*this, std::forward<Args>(args)..., obj->exportLocal((Endpoint&)*this, obj), id);
    	return f;
    }

    template<class Ret, class Call, class Obj, class... Args>
    inline std::future<Ret> createWithPromiseRetval(Call& call, Obj obj, Args&&... args)
    {
    	std::promise<Ret> p;
		auto f = p.get_future();

    	auto id = this->install([p{std::move(p)}, obj](Endpoint& rpc, rpc::MethodHandle h, Ret arg, rpc::Arg<0, &rpc::remove_cref_t<decltype(*obj)>::importRemote> import) mutable
		{
    		obj->importRemote(import);
    		p.set_value(rpc::move(arg));
    		rpc.uninstall(h);
    	});

    	call.call(*this, std::forward<Args>(args)..., obj->exportLocal((Endpoint&)*this, obj), id);
    	return f;
    }

public:
    using Endpoint = typename ClientBase::StlEndpoint::Endpoint;

    using Endpoint::call;
    using Endpoint::install;
    using Endpoint::uninstall;
    using Endpoint::lookup;
    using Endpoint::provide;
    using Endpoint::discard;
    using Endpoint::process;

    using ClientBase::StlEndpoint::StlEndpoint;
};

}

#endif /* RPC_CPP_INTEROP_RPCCLIENT_H_ */

#ifndef RPC_CPP_INTEROP_RPCCLIENT_H_
#define RPC_CPP_INTEROP_RPCCLIENT_H_

#include "RpcFail.h"

#include <mutex>
#include <condition_variable>

namespace rpc {

/**
 * Facade for the generic RPC endpoint using STL based implementation of auxiliary operations.
 *
 * This is supposed to be the regular frontend class for PC-like environments - i.e. wherever
 * using the STL classes is advisable. This also means that dynamic memory usage is managed by
 * the STL implementation. When tighter control ver heap usage is a requirement alternate
 * implementations for the dependencies can be used.
 */
template<class Io>
class ClientBase:
	public Io,
	public Endpoint<
		detail::StlAutoPointer,
		detail::HashMapRegistry,
		typename Io::InputAccessor,
		ClientBase<Io>
	>
{
    friend typename ClientBase::Endpoint;
	friend Io;

    volatile bool locked;
    std::mutex m;
    std::condition_variable cv;

public:
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

    using Endpoint = typename ClientBase::Endpoint;

    /**
     * Wrapper around Endpoint::init.
     */
    template<class... Args>
    ClientBase(Args&&... args): Io(std::forward<Args>(args)...)
	{
    	if(!this->Endpoint::init())
    	{
    		fail("could not initialize RPC client object");
    	}
    }
};

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
	auto call(Rpc& rpc, Args&&... args)
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
					fail(std::string("failed to look up symbol") + (const char*)sym);
				}
			});
		}
	}
};

template<class Sym> OnDemand(const Sym &sym) -> OnDemand<Sym>;

}

#endif /* RPC_CPP_INTEROP_RPCCLIENT_H_ */

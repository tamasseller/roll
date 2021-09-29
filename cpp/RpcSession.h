#ifndef RPC_CPP_RPCSESSION_H_
#define RPC_CPP_RPCSESSION_H_

#include "RpcFail.h"
#include "RpcErrors.h"

namespace rpc {

template<class> class ClientBase;

template<class Imported, class Exported, size_t nExported>
class SessionBase
{
	template<class> friend class rpc::ClientBase;
	using OnClosed = rpc::Call<>;

	Exported exported;
	Imported imported;
	bool importDone = false;

	using Unexporter = void (*)(void*, const void*);
	Unexporter unexporters[nExported + 1] = {nullptr, };

	template<auto method, class Ep, class Self>
	static inline void unexportCall(void* endpunkt, const void* mich)
	{
		auto& self = *static_cast<const Self*>(mich);
		auto& ep = *static_cast<Ep*>(endpunkt);
		ep.uninstall(((*self).exported).*method);
	}

	template<class Self>
	static inline void callOnClosed(void*, const void* mich)
	{
		auto& self = *static_cast<const Self*>(mich);
		(*self).onClosed();
	}

	inline bool addFinalizer(Unexporter u)
	{
		for(auto &v: this->unexporters)
		{
			if(v == nullptr)
			{
				v = u;
				return true;
			}
		}

		return false;
	}

	inline void finalize(void* ep, const void* me)
	{
		for(auto &u: unexporters)
		{
			u(ep, me);
			u = nullptr;
		}
	}

protected:
	template<auto method, class Ep, class... Args>
	inline auto callImported(Ep& _ep, Args&&... args)
	{
        if(!importDone)
        {
        	rpc::fail(rpc::Errors::sessionNotOpen);
        }

        if(auto _err = _ep->call(imported.*method, std::forward<Args>(args)...))
		{
        	rpc::fail(_err);
		}
	}

	template<auto method, auto onClosedMember, class Ep, class Self, class... Args>
	inline auto exportCall(Ep& _ep, Self _self)
	{
		if(!addFinalizer(&unexportCall<method, Ep, Self>))
		{
			rpc::fail("Finalizer table full");
		}
		else
		{
			exported.*method = _ep.install([_self](Ep& ep, rpc::MethodHandle, Args... args) {
				((*_self).*onClosedMember)(std::forward<Args>(args)...);
			});
		}
	}

	template<auto onClosedMember, class Ep, class Self>
	inline auto finalizeExports(Ep& _ep, Self _self)
	{
		if(!addFinalizer(&callOnClosed<Self>))
		{
			rpc::fail("Finalizer table full");
			return *(decltype(exported)*)nullptr;
		}
		else
		{
			exported._close = _ep.install([_self](Ep& ep, rpc::MethodHandle h)
			{
				(*_self).finalize(&ep, &_self);
				ep.uninstall(h);
			});

			return exported;
		}
	}

public:
	inline void importRemote(const Imported& imported)
	{
		this->imported = imported;
		importDone = true;
	}

	template<class Ep>
	inline auto close(Ep& _ep) {
		return callImported<&Imported::_close>(_ep);
	}

	inline ~SessionBase()
	{
		for(auto u: unexporters)
		{
			if(u)
			{
				rpc::fail("Active session destroyed");
				break;
			}
		}
	}
};

}

#endif /* RPC_CPP_RPCSESSION_H_ */

#ifndef RPC_CPP_RPCSESSION_H_
#define RPC_CPP_RPCSESSION_H_

#include "common/Errors.h"

#include "Fail.h"
#include "Tracker.h"

namespace rpc {

template<class> class ClientBase;

template<class Imported, class Exported, size_t nExported>
class SessionBase: public Tracker::Subobject
{
	template<class> friend class rpc::ClientBase;
	using Unexporter = void (*)(void*, const void*);

	Exported exported;
	Imported imported;
	bool importDone = false;
	Unexporter unexporters[nExported + 1] = {nullptr, };

	template<auto method, class Ep, class Self>
	static inline void unexportCall(void* e, const void* s)
	{
		auto& self = *static_cast<const Self*>(s);
		auto& ep = *static_cast<Ep*>(e);
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

	template<bool ignoreFailure, auto method, class Ep, class... Args>
	inline void callImportedMayFail(const Ep& ep, Args&&... args)
	{
        if(!importDone)
        {
			if constexpr(ignoreFailure)
			{
				return;
			}
			else
			{
				rpc::fail("the session is not functional (yet/anymore)"); /* GCOV_EXCL_LINE */
			}
        }

        auto r = ep->call(imported.*method, rpc::forward<Args>(args)...);

		if constexpr(!ignoreFailure)
		{
			if(!!r)
			{
				rpc::fail(rpc::getErrorString(r)); /* GCOV_EXCL_LINE */
			}
		}
		else
		{
			(void)r;
		}
	}

protected:
	template<auto method, class Ep, class... Args>
	inline void callImported(const Ep& ep, Args&&... args) {
        callImportedMayFail<false, method, Ep, Args...>(ep, rpc::forward<Args>(args)...);
	}

	template<auto method, auto member, class Ep, class Self, class... Args>
	inline auto exportCall(Ep& ep, Self self)
	{
		if(!addFinalizer(&unexportCall<method, Ep, Self>))
		{
			rpc::fail("Finalizer table full"); /* GCOV_EXCL_LINE */
		}
		else
		{
			exported.*method = ep.install([self](Ep& ep, rpc::MethodHandle, Args... args) {
				((*self).*member)(rpc::forward<Args>(args)...);
			});
		}
	}

	template<auto onClosedMember, class Ep, class Self>
	inline auto finalizeExports(Ep& ep, Self self)
	{
		if(!addFinalizer(&callOnClosed<Self>))
		{
			rpc::fail("Finalizer table full"); /* GCOV_EXCL_LINE */
			return *(decltype(exported)*)nullptr;
		}
		else
		{
			exported._close = ep.install([self](Ep& ep, rpc::MethodHandle h)
			{
				(*self).finalize(&ep, &self);
				ep.Tracker::removeSubobject(self->Tracker::Subobject::asSubobject());
				ep.uninstall(h);
			});

			ep.Tracker::addSubobject(self->Tracker::Subobject::asSubobject(), exported._close);

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
	inline void close(const Ep& ep)
	{
		if(importDone)
		{
			callImportedMayFail<true, &Imported::_close, Ep>(ep);
			importDone = false;
		}
	}

	inline ~SessionBase()
	{
		for(auto u: unexporters)
		{
			if(u)
			{
				rpc::fail("Active session destroyed"); /* GCOV_EXCL_LINE */
				break;
			}
		}
	}
};

}

#endif /* RPC_CPP_RPCSESSION_H_ */

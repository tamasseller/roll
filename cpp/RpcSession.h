#ifndef RPC_CPP_RPCSESSION_H_
#define RPC_CPP_RPCSESSION_H_

#include "RpcClient.h"

template<class Imported, class Exported>
struct SessionBase
{
	template<class> friend class rpc::ClientBase;

	Imported imported;
	Exported exported;
	bool importDone = false, exportsActive = false;

	inline void importRemote(const Imported& imported)
	{
		this->imported = imported;
		importDone = true;
	}

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

	template<auto method, auto member, class Ep, class Self, class... Args>
	inline auto exportCall(Ep& _ep, Self _self)
	{
		exported.*method = _ep.install([_self](Ep& ep, rpc::MethodHandle, Args... args) {
			((*_self).*member)(std::forward<Args>(args)...);
		});

		exportsActive = true;
	}
};

#endif /* RPC_CPP_RPCSESSION_H_ */

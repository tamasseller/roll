#ifndef _INTEROPTEST_H_
#define _INTEROPTEST_H_

#include "RpcClient.h"
#include "RpcSession.h"
#include "RpcFdStreamAdapter.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"

#include "Contract.gen.h"

#include <memory>

using Client = InteropTestContract::ClientProxy<rpc::FdStreamAdapter>;

template<class Child, class Adapter> struct XXX: InteropTestContract::ServerProxy<Child, Adapter>
{
	template<class... Args>
	inline XXX(Args&&... args): XXX::ServerProxy(std::forward<Args>(args)...)
	{
		{
			static_assert(rpc::nArgs<&Child::open> == 2, "Session constructor open for stream session takes 2 arguments");
			static_assert(rpc::isCompatible<rpc::Arg<0, &Child::open>, uint8_t>(), "Argument #1 of session constructor open for stream session must have type compatible with 'u1'");
			static_assert(rpc::isCompatible<rpc::Arg<1, &Child::open>, uint8_t>(), "Argument #2 of session constructor open for stream session must have type compatible with 'u1'");

			using RetType = typename rpc::Ret<&Child::open>;
			static constexpr bool retValOk = rpc::isCompatible<decltype(std::declval<RetType>().first), rpc::CollectionPlaceholder<int8_t>>();
			static constexpr bool objectOk = rpc::hasCrtpBase<InteropTestStreamServerSession, decltype(*std::declval<RetType>().second)>;
			static_assert(retValOk && objectOk, "Session constructor open for stream session must return a pointer-like object to a CRTP subclass of InteropTestContract::StreamServerSession");

			auto _err = this->provide(InteropTestContract::Symbols::StreamSession::symOpen, [self{static_cast<Child*>(this)}](typename XXX::Endpoint& _ep, rpc::MethodHandle, rpc::Arg<0, &Child::open> initial, rpc::Arg<1, &Child::open> modulus, InteropTestContract::Types::StreamSession::StreamCallbackExports _exports, InteropTestContract::Types::StreamSession::OpenAccept _cb)
			{
				auto _pair= self->open(std::move(initial), std::move(modulus));
				auto _ret = std::move(_pair.first);
				auto _obj = std::move(_pair.second);

				_obj->importRemote(_exports);

				if(auto _err = _ep.call(_cb, std::move(_ret), _obj->exportLocal(_ep, _obj)))
				{
					rpc::fail(std::string("Calling callback of public method 'echo' resulted in error ") + _err);
				}
			});

			if(_err)
			{
				rpc::fail(std::string("Registering public method 'echo' resulted in error ") + _err);
			}
		}

		{
			static_assert(rpc::nArgs<&Child::openDefault> == 0, "Session constructor openDefault for stream session takes 0 argument");
			static_assert(rpc::hasCrtpBase<InteropTestStreamServerSession, decltype(*std::declval<rpc::Ret<&Child::openDefault>>())>, "Session constructor openDefault for stream session must return a pointer-like object to a CRTP subclass of InteropTestContract::StreamServerSession");

			auto _err = this->provide(InteropTestContract::Symbols::StreamSession::symOpenDefault, [self{static_cast<Child*>(this)}](typename XXX::Endpoint& _ep, rpc::MethodHandle, InteropTestContract::Types::StreamSession::StreamCallbackExports _exports, InteropTestContract::Types::StreamSession::OpenDefaultAccept _cb)
			{
				auto _obj = self->openDefault();

				_obj->importRemote(_exports);

				if(auto _err = _ep.call(_cb, _obj->exportLocal(_ep, _obj)))
				{
					rpc::fail(std::string("Calling callback of public method 'echo' resulted in error ") + _err);
				}
			});

			if(_err)
			{
				rpc::fail(std::string("Registering public method 'echo' resulted in error ") + _err);
			}
		}
	}

};

void runInteropTests(int sock);
void runInteropListener(int sock);

#endif /* _INTEROPTEST_H_ */

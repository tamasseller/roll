#ifndef _RPCENDPOINT_H_
#define _RPCENDPOINT_H_

#include "RpcCore.h"
#include "RpcCTStr.h"
#include "RpcErrors.h"
#include "RpcSymbol.h"
#include "RpcArrayWriter.h"
#include "RpcStreamReader.h"
#include "RpcSignatureGenerator.h"

namespace rpc {

namespace detail 
{
	template<class> struct CallOperatorSignatureUtility;
	template<class T> using Plain = remove_const_t<remove_reference_t<T>>;

	template<class Ret, class Type, class Ctx1, class Ctx2, class... Args> struct CallOperatorSignatureUtility<Ret (Type::*)(Ctx1, Ctx2, Args...) const>
	{
		template<class Core, class C>
		static inline decltype(auto) install(Core &core, C&& c) {
			return Call<Plain<Args>...>{core.template add<Plain<Args>...>(rpc::forward<C>(c))};
		}
	};
}

/**
 * RPC engine frontend.
 */
template <
	template<class> class Pointer,
	template<class, class> class Registry,
	class NameRegistry,
	class IoEngine
>
class Endpoint:
	Core<
		typename IoEngine::Factory, 
		Pointer, 
		Registry, 
		Endpoint<Pointer, Registry, NameRegistry, IoEngine>&>, 
	NameRegistry, public IoEngine
{
	using Accessor = typename Endpoint::Core::Accessor;
	using CallId = typename Endpoint::Core::CallId;
	static constexpr CallId lookupId = 0, invalidId = -1u;

	const char* doLookup(const char* name, size_t length, CallId cb)
	{
		bool buildOk;

		auto data = this->Endpoint::Core::template buildCall<ArrayWriter<char>, Call<CallId>>(
			buildOk, lookupId, ArrayWriter<char>(name, length), Call<CallId>{cb});	

		if(buildOk)
		{
			if(IoEngine::send(rpc::move(data)))
				return nullptr;

			return Errors::couldNotSendLookupMessage;
		}

		return Errors::couldNotCreateLookupMessage;
	}

public:
	using IoEngine::IoEngine;

	/**
	 * Initialize the internal state of the RPC endpoint.
	 * 
	 * NOTE: Must be called before any other member.
	 */
	bool init()
	{
		return this->Endpoint::Core::template addCallAt<StreamReader<char, Accessor>, Call<CallId>>(lookupId, []
		(Endpoint& ep, const MethodHandle &id, StreamReader<char, Accessor> name, Call<CallId> callback)
		{
			auto nameQuery = ep.NameRegistry::beginQuery();

			for(char c: name)
				nameQuery << c;

			CallId result = invalidId;
			const char* ret = nullptr;

			if(!nameQuery.run(ep, result))
				ret = Errors::unknownMethodRequested;

			if(auto err = ep.call(callback, result))
				return err;

			return ret;
		});
	}

	/**
	 * Process an incoming message.
	 * 
	 * Executes a registered method as requested by the message.
	 * 
	 * Returns true on success, false if there was an error during processing. 
	 * The possible errors include:
	 * 
	 *  - Parse error,
	 *  - IO error during reading or
	 *  - Failure to find the requested method in the registry.
	 */
	auto process(Accessor& a) {
		return Endpoint::Core::execute(a, *this);
	}

	/**
	 * Register a private RPC method for remote execution.
	 * 
	 * The registration created this way is not public in the sense that it 
	 * can not be looked up by the remote end using a name. However it can 
	 * be used for callbacks by sending the returned handle over to the 
	 * remote end that can use it later to invoke the registered method.
	 * 
	 * The argument must be a functor object that accepts the following arguments:
	 * 
	 *   - first: a reference to the Endpoint object, that can be used to carry out further operations,
	 *   - second: an rpc::MethodHandle opaque identifier that can be used to remove the
	 *     registration (for example via the reference to the Endpoint object in the first 
	 *     argument) during execution of the method itself,
	 *   - the rest of the arguments are the ones received from the remote call the types of which are
	 *     captured in the returned Call object.
	 * 
	 * Returns a Call method handle that can be used to execute the registered method.
	 */
	template<class C>
	inline auto install(C&& c) 
	{
		auto &core = *((typename Endpoint::Core*)this);
		return detail::CallOperatorSignatureUtility<decltype(&C::operator())>::install(core, rpc::move(c));
	}

	/**
	 * Removes a method registration identified by an opaque handle supplied to the 
	 * method at invocation.
	 * 
	 * Returns nullptr on success, the appropriate rpc::Errors constants string member on error.
	 */
	inline const char* uninstall(const rpc::MethodHandle &h)
	{
		auto &nameReg = *((NameRegistry*)this);
		nameReg.removeMapping(h.id);

		auto &core = *((typename Endpoint::Core*)this);

		if(!core.removeCall(h.id))
			return Errors::methodNotFound;

		return nullptr;
	}

	/**
	 * Removes a method registration identified by its Call object returned at 
	 * registration.
	 * 
	 * Returns nullptr on success, the appropriate rpc::Errors constants string member on error.
	 */
	template<class... Args>
	inline auto uninstall(const rpc::Call<Args...> &c) {
		return uninstall(rpc::MethodHandle(c));
	}

	/**
	 * Initiates a remote method call with the supplied parameters.
	 * 
	 * Returns nullptr on success, the appropriate rpc::Errors constants string member on error.
	 * The possible errors include:
	 * 
	 *  - Processign error during the serialization of the method identifier or arguments,
	 *  - IO error during sending the request.
	 */
	template<class... NominalArgs, class... ActualArgs>
	inline const char* call(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
		bool buildOk;
		auto data = this->Endpoint::Core::template buildCall<NominalArgs...>(buildOk, call.id, rpc::forward<ActualArgs>(args)...);	
		if(buildOk)
		{
			if(IoEngine::send(rpc::move(data)))
				return nullptr;

			return Errors::couldNotSendMessage;
		}
		else
		{
			return Errors::couldNotCreateMessage;
		}
	}

	/**
	 * Provide a publicly accessible method.
	 * 
	 * Registers the implementation of a public method identified by a Symbol object, 
	 * that can be looked up by the remote end.
	 * 
	 * The arguments of the functor are expected to be the same as described for the _install_ operation.
	 *
	 * Returns nullptr on success, the appropriate rpc::Errors constants string member on error.
	 */
	template<size_t n, class... Args, class C>
	inline const char* provide(Symbol<n, Args...> sym, C&& c) 
	{
		auto &core = *((typename Endpoint::Core*)this);
		auto id = core.template add<detail::Plain<Args>...>(rpc::forward<C>(c));
		
		auto &nameReg = *((NameRegistry*)this);
		if(nameReg.addMapping((const char*)sym, id))
			return nullptr;
		
		if(!core.removeCall(id))
			return Errors::internalError; // GCOV_EXCL_LINE
		
		return Errors::symbolAlreadyExported;
	}

	/**
	 * Discard a publicly accessible method.
	 * 
	 * Removes a previously registered method identified by the provided Symbol object. 
	 * After the operation the method can no longer be looked up or invoked using a handle
	 * acquired earlier.
	 * 
	 * Returns nullptr on success, the appropriate rpc::Errors constants string member on error.
	 */
	template<size_t n, class... Args>
	inline const char* discard(Symbol<n, Args...> sym) 
	{
		CallId id;
		
		auto &nameReg = *((NameRegistry*)this);
		if(!nameReg.removeMapping((const char*)sym, id))
			return Errors::noSuchSymbol;

		auto &core = *((typename Endpoint::Core*)this);
		if(!core.removeCall(id))
			return Errors::internalError; // GCOV_EXCL_LINE
		
		return nullptr;
	}

	/**
	 * Lookup public remote method.
	 * 
	 * A request is sent based on the a Symbol object provided, the result is passed
	 * to the supplied functor when a reply is received. The functor is expected to
	 * receive two arguments:
	 * 
	 *   - first: a reference to the Endpoint object, that can be used to initiate
	 *     further operations on the obtained result,
	 *   - second: a bool value, which is true if the reply indicates success,
	 *   - third: Call object that can be used to invoke the looked up remote method
	 *     if the operation was successful (indicated by the first argument). If the
	 * 	   reply indicates failure - probably because the queried symbol could not be 
	 *     found remotely - then the value of the second argument must be considered
	 *     invalid and not be used for remote invocation.
	 *      
	 * Returns nullptr on success, the appropriate rpc::Errors constants string member on error.
	 */
	template<size_t n, class... Args, class C>
	inline const char* lookup(const Symbol<n, Args...> &sym, C&& c) 
	{
		auto &core = *((typename Endpoint::Core*)this);
		auto id = core.template add<CallId>([this, c{rpc::forward<C>(c)}](Endpoint &uut, const rpc::MethodHandle &handle, CallId result) mutable
		{
			c(uut, result != invalidId, Call<Args...>{result});

			auto &core = *((typename Endpoint::Core*)this);
			if(!core.removeCall(handle.id))
				return Errors::internalError; // GCOV_EXCL_LINE

			return (const char*) nullptr;
		});

		if(auto err = doLookup((const char*)sym, n, id))
		{
			if(!core.removeCall(id))
				return Errors::internalError; // GCOV_EXCL_LINE

			return err;
		}
		
		return nullptr;
	}
};

}

#endif /* _RPCENDPOINT_H_ */

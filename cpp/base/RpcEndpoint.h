#ifndef _RPCENDPOINT_H_
#define _RPCENDPOINT_H_

#include "common/CTStr.h"
#include "common/Errors.h"

#include "types/CallTypeInfo.h"
#include "types/PrimitiveTypeInfo.h"

#include "Symbol.h"
#include "Serdes.h"
#include "SignatureGenerator.h"

namespace rpc {

struct EmptyBase {};

/**
 * RPC engine front-end.
 */
template <
	class Child,
	template<class> class Pointer,
	template<class, class> class Registry,
	class InputAccessor,
	class IoEngine,
	class RegistryElementBase = EmptyBase
>
class Endpoint
{
	using CallId = uint32_t;

	template<class> struct CallOperatorSignatureUtility;

	template<class Ret, class Type, class Ctx1, class Ctx2, class... Args> struct CallOperatorSignatureUtility<Ret (Type::*)(Ctx1, Ctx2, Args...) const>
	{
		template<class Core, class C>
		static inline decltype(auto) install(Core &core, C&& c) {
			return Call<remove_cref_t<Args>...>{core.template add<remove_cref_t<Args>...>(rpc::forward<C>(c))};
		}
	};

	template<class Ret, class Type, class Ctx1, class Ctx2, class... Args> struct CallOperatorSignatureUtility<Ret (Type::*)(Ctx1, Ctx2, Args...)>:
		CallOperatorSignatureUtility<Ret (Type::*)(Ctx1, Ctx2, Args...) const> {};

	/**
	 * Polymorphic invocation interface.
	 */
	struct IInvoker: RegistryElementBase {
		/**
		 * Deserialization and invocation of a registered method.
		 */
		virtual Errors invoke(InputAccessor &a, CallId id, Child& ep) = 0;

		/**
		 * The virtual destructor is required because the captured
		 * functor may have non-trivial destructor.
		 */
		inline virtual ~IInvoker() = default;
	};

	/**
	 * Templated implementation of the invocation interface.
	 */
	template<class T, class... NominalArgs>
	struct Invoker: IInvoker
	{
		/**
		 * The captured RPC method functor.
		 */
		T target;

		/**
		 * The actual invoker object is always move constructed to
		 * allow for move-only functor type (for example a lambda
		 * object that captures a move-only value).
		 */
		inline Invoker(T&& target): target(rpc::move(target)) {}

		/**
		 * The virtual destructor is required because the captured
		 * functor may have non-trivial destructor. (for example
		 * captured in a lambda).
		 */
		inline virtual ~Invoker() = default;

		/**
		 * Implementation of the specific deserialization and invocation.
		 * Knowns the argument types and the actual target functor to be called.
		 * Uses deserializer helper to parse the arguments and pass them directly
		 * to the target method.
		 */
		inline virtual Errors invoke(InputAccessor &a, CallId id, Child& ep) override final {
			return deserialize<NominalArgs...>(a, target, ep, MethodHandle(id));
		}
	};

	/**
	 * Numeric identifier based RPC method registry.
	 */
	Registry<CallId, Pointer<IInvoker>> registry;

	Registry<decltype(""_ctstr.hash()), CallId> symbolRegistry;

	/**
	 * Helper to assign the next free id to a new registration.
	 */
	CallId maxId = 0;

	/**
	 * Process an incoming message.
	 *
	 * The call identifier is parsed from the beginning, then the method
	 * registry is consulted to find the appropriate invoker object. The
	 * parsing of and the actual invocation is handled by the invoker.
	 *
	 * Return true on success, false on error. The errors may invlude:
	 *
	 *   - Parse error during method identifier or argument parsing.
	 *   - Failure to find the method registration corresponding to the identifier.
	 */
	inline Errors execute(CallId id, InputAccessor &a)
	{
		bool ok;
		auto it = registry.find(id, ok);

		if(!ok)
		{
			return Errors::undefinedMethodCalled;
		}

		return (*it)->invoke(a, id, *static_cast<Child*>(this));
	}

	/**
	 * Register a method for any available method identifier.
	 *
	 * Returns the associated method identifier.
	 */
	template<class... Args, class T>
	inline CallId add(T&& call)
	{
		auto ptr = Pointer<IInvoker>::template make<Invoker<T, Args...>>(rpc::move(call));

		CallId id;

		do
		{
			id = maxId++;
		}
		while(!registry.add(id, rpc::move(ptr)));

		return id;
	}

	/**
	 * Build a message for invoking a method with the provided identifier
	 * and arguments. Arguments are serialized using the serialize helper
	 * according to the rules specified by the TypeInfo template class.
	 */
	template<class... NominalArgs, class... ActualArgs, class Factory>
	static inline auto buildCall(Factory& factory, bool &ok, CallId id, ActualArgs&&... args)
	{
		using C = Call<NominalArgs...>;
		C c{id};

		static_assert(writeSignature<NominalArgs...>(""_ctstr) == writeSignature<ActualArgs...>(""_ctstr), "RPC invocation signature mismatched");

		auto size = determineSize(c, args...);
		auto pdu = factory.build(size);

		ok = serialize(pdu, c, rpc::forward<ActualArgs>(args)...);
		return factory.done(rpc::move(pdu));
	}

	Errors doLookup(uint64_t id, size_t length, CallId cb)
	{
		bool buildOk;

		auto f = static_cast<IoEngine*>(this)->messageFactory();

		auto data = buildCall<uint64_t, Call<CallId>>
		(
			f,
			buildOk,
			lookupId,
			id,
			Call<CallId>{cb}
		);

		if(buildOk)
		{
			if(static_cast<IoEngine*>(this)->send(rpc::move(data)))
			{
				return Errors::success;
			}

			return Errors::couldNotSendLookupMessage;
		}

		return Errors::couldNotCreateLookupMessage;
	}

public:
	static constexpr CallId lookupId = 0, invalidId = -1u;

	/**
	 * Initialize the internal state of the RPC endpoint.
	 * 
	 * NOTE: Must be called before any other member.
	 */
	bool init()
	{
		auto lookupResponder = [](Child& ep, const MethodHandle &id, uint64_t idHash, Call<CallId> callback)
		{
			CallId r = invalidId;
			Errors ret = Errors::success;

			bool ok;
			auto result = ep.symbolRegistry.find(idHash, ok);
			if(ok)
			{
				r = *result;
			}
			else
			{
				ret = Errors::unknownSymbolRequested;
			}

			if(auto err = ep.call(callback, r); !!err)
			{
				ret = err;
			}

			return ret;
		};

		auto respInvoker = Pointer<IInvoker>::template make<Invoker<decltype(lookupResponder), uint64_t, Call<CallId>>>(rpc::move(lookupResponder));

		return registry.add(lookupId, rpc::move(respInvoker));
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
	auto process(InputAccessor& a)
	{
		CallId id;

		if(!VarUint4::read(a, id))
		{
			return Errors::messageFormatError;
		}

		return execute(id, a);
	}

	/**
	 * Register a private RPC method for remote execution.
	 * 
	 * The registration created this way is not public in the sense that it 
	 * can not be looked up by the remote end using a name. However it can 
	 * be used to implements a callback by sending the returned handle over
	 * to the remote end that can use it later to invoke the registered method.
	 * 
	 * The argument must be a functor object that accepts the following arguments:
	 * 
	 *   - first: a reference to the Endpoint object, that can be used to carry out further operations,
	 *   - second: an rpc::MethodHandle opaque identifier that can be used to remove the
	 *     registration (for example via the reference to the Endpoint object in the first 
	 *     argument) during execution of the method itself,
	 *   - the rest of the arguments are the ones received from the remote call, the types of which are
	 *     captured in the returned Call object.
	 * 
	 * Returns a Call method handle that can be used to execute the registered method.
	 */
	template<class C>
	inline auto install(C&& c) {
		return CallOperatorSignatureUtility<decltype(&C::operator())>::install(*this, rpc::move(c));
	}

	/**
	 * Removes a method registration identified by an opaque handle supplied to the 
	 * method at its invocation by the RPC engine.
	 * 
	 * NOTE: a public method must not be **uninstalled** this way, but it needs to be **discarded**
	 *       using its symbol instead.
	 *
	 * The returned error code indicates success or the type of failure that occurred.
	 */
	inline Errors uninstall(const rpc::MethodHandle &h)
	{
		if(!registry.remove(h.id))
		{
			return Errors::methodNotFound;
		}

		return Errors::success;
	}

	/**
	 * Removes a method registration identified by its Call object returned at 
	 * registration.
	 * 
	 * The returned error code indicates success or the type of failure that occurred.
	 */
	template<class... Args>
	inline auto uninstall(const rpc::Call<Args...> &c) {
		return uninstall(rpc::MethodHandle(c));
	}

	/**
	 * Initiate a remote method call with the supplied parameters.
	 * 
	 * The returned error code indicates success or the type of failure that occurred.
	 * The possible errors include:
	 * 
	 *  - Processing error during the serialization of the method identifier or arguments,
	 *  - IO error during sending the request.
	 */
	template<class... NominalArgs, class... ActualArgs>
	inline Errors call(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
		bool buildOk;

		auto f = static_cast<IoEngine*>(this)->messageFactory();

		auto data = this->Endpoint::template buildCall<NominalArgs...>(f, buildOk, call.id, rpc::forward<ActualArgs>(args)...);
		if(buildOk)
		{
			if(static_cast<IoEngine*>(this)->send(rpc::move(data)))
				return Errors::success;

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
	 * The returned error code indicates success or the type of failure that occurred.
	 */
	template<size_t n, class... Args, class C>
	inline Errors provide(const Symbol<n, Args...> &sym, C&& c)
	{
		Call<Args...> id = this->install(rpc::forward<C>(c));
		
		if(!symbolRegistry.add(sym.hash(), rpc::move(id.id)))
		{
			if(!registry.remove(id.id))
			{
				return Errors::internalError; // GCOV_EXCL_LINE
			}

			return Errors::symbolAlreadyExported;
		}

		return Errors::success;
	}

	/**
	 * Discard a publicly accessible method.
	 * 
	 * Removes a previously registered method identified by the provided Symbol object. 
	 * After the operation the method can no longer be looked up or invoked using a handle
	 * acquired earlier.
	 * 
	 * The returned error code indicates success or the type of failure that occurred.
	 */
	template<size_t n, class... Args>
	inline Errors discard(const Symbol<n, Args...> &sym)
	{
		const auto idHash = sym.hash();

		bool ok;
		auto rPtr = symbolRegistry.find(idHash, ok);

		if(!ok)
		{
			return Errors::symbolNotFound;
		}

		auto result = *rPtr;

		if(!symbolRegistry.remove(idHash) || !registry.remove(result))
		{
			return Errors::internalError; // GCOV_EXCL_LINE
		}
		
		return Errors::success;
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
	 * The returned error code indicates success or the type of failure that occurred.
	 */
	template<size_t n, class... Args, class C>
	inline Errors lookup(const Symbol<n, Args...> &sym, C&& c)
	{
		auto id = add<CallId>([this, c{rpc::forward<C>(c)}](Child &ep, const rpc::MethodHandle &handle, CallId result) mutable
		{
			c(ep, result != invalidId, Call<Args...>{result});

			if(!registry.remove(handle.id))
			{
				return Errors::internalError; // GCOV_EXCL_LINE
			}

			return Errors::success;
		});

		if(auto err = doLookup(sym.hash(), n, id); !!err)
		{
			if(!registry.remove(id))
			{
				return Errors::internalError; // GCOV_EXCL_LINE
			}

			return err;
		}
		
		return Errors::success;
	}

	/**
	 * Issues a simulated call to a locally registered method that takes no arguments.
	 */
	inline Errors simulateCall(rpc::Call<> call) {
		return execute(call.id, *((InputAccessor*)nullptr));
	}
};

}

#endif /* _RPCENDPOINT_H_ */

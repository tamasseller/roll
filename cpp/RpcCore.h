#ifndef _RPCCORE_H_
#define _RPCCORE_H_

#include "RpcSerdes.h"

namespace rpc {

/**
 * RPC Call dispatcher.
 * 
 * It implements:
 *   - message serialization and parsing,
 *   - call dispatching,
 *   - id based method registry management.
 */
template
<
	class InputAccessor,
	template<class> class Pointer,
	template<class, class> class Registry,
	class... ExtraArgs
>
class Core
{
protected:
	using CallId = uint32_t;

private:
	/**
	 * Polymorphic invocation interface.
	 */
	struct IInvoker {
		/**
		 * Deserialization and invocation of a registered method.
		 */
		virtual const char* invoke(InputAccessor &a, CallId id, ExtraArgs...) = 0;

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
		Invoker(T&& target): target(rpc::move(target)) {}

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
		virtual const char* invoke(InputAccessor &a, CallId id, ExtraArgs... extraArgs) override {
			return deserialize<NominalArgs...>(a, target, extraArgs..., MethodHandle(id));
		}
	};

	/**
	 * Numeric identifier based RPC method registry.
	 */
	Registry<CallId, Pointer<IInvoker>> registry;

	/**
	 * Helper to assign the next free id to a new registration. 
	 */
	CallId maxId = 0;

public:
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
	const char* execute(InputAccessor &a, ExtraArgs... args)
	{
		CallId id;

		if(!VarUint4::read(a, id))
			return Errors::messageFormatError;

		bool ok;
		auto it = registry.find(id, ok);
		if(!ok)
			return Errors::wrongMethodRequest;

		return (*it)->invoke(a, id, args...);
	}

	/**
	 * Register a method with a certain method identifier.
	 * 
	 * This method can be used to add an implementation for a well-known identifier.
	 * 
	 * Returns true on success, false if the specified identifier is already taken.
	 */
	template<class... Args, class T>
	inline bool addCallAt(CallId id, T&& call) {
		return registry.add(id, Pointer<IInvoker>::template make<Invoker<T, Args...>>(rpc::move(call)));
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
	 * Remove association of with a method and identifier.
	 * 
	 * Returns true on success, false if there was 
	 * registered method for the given identifier.
	 */
	inline bool removeCall(uint32_t id) {
		return registry.remove(id);
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
};

}

#endif /* _RPCCORE_H_ */

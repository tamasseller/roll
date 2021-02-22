#ifndef COMMON_RPCCORE_H_
#define COMMON_RPCCORE_H_

#include "RpcSerdes.h"

namespace rpc {

template
<
	class StreamWriterFactory, 
	template<class> class Pointer,
	template<class, class> class Registry
>
class Core
{
	using Accessor = typename StreamWriterFactory::Accessor;
	using CallId = uint32_t;

	struct IInvoker {
		virtual bool invoke(Accessor &a) = 0;
		inline virtual ~IInvoker() = default;
	};

	template<class T, class... NominalArgs>
	struct Invoker: IInvoker
	{
		T target;
		Invoker(T&& target): target(rpc::move(target)) {}
		inline virtual ~Invoker() = default;

		virtual bool invoke(Accessor &a) override {
			return deserialize<NominalArgs...>(a, target);
		}
	};

	Registry<CallId, Pointer<IInvoker>> registry;
	CallId maxId = 0;

public:
	inline ~Core() {
	}

	bool execute(Accessor &a)
	{
		CallId id;

		if(!VarUint4::read(a, id))
			return false;

		bool ok;
		auto it = registry.find(id, ok);
		if(!ok)
			return false;

		return (*it)->invoke(a);
	}

	template<class... Args, class T>
	inline bool addCallAt(CallId id, T&& call) {
		return registry.add(id, Pointer<IInvoker>::template make<Invoker<T, Args...>>(rpc::move(call)));
	}

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

	inline bool removeCall(uint32_t id) {
		return registry.remove(id);
	}

	template<class... NominalArgs, class... ActualArgs>
	static inline auto buildCall(CallId id, ActualArgs&&... args)
	{
		using C = Call<NominalArgs...>;
		C c{id};

		auto size = determineSize<const C&, NominalArgs...>(c, rpc::forward<ActualArgs>(args)...);
		auto pdu = StreamWriterFactory::build(size);
		bool serOk = serialize<const C&, NominalArgs...>(pdu, c, rpc::forward<ActualArgs>(args)...);
		// assert(serOk);
		return StreamWriterFactory::done(rpc::move(pdu));
	}
};

}

#endif /* COMMON_RPCCORE_H_ */

#ifndef _RPCENDPOINT_H_
#define _RPCENDPOINT_H_

#include "RpcCore.h"
#include "RpcStreamReader.h"
#include "RpcArrayWriter.h"
#include "RpcSignatureGenerator.h"

#include <unordered_map>

namespace rpc {

namespace detail 
{
	template<class> struct CallOperatorSignatureUtility;

	template<class Type, class... Args> struct CallOperatorSignatureUtility<void (Type::*)(Args...) const>
	{
		template<class T> using Plain = remove_const_t<remove_reference_t<T>>;

		template<class Core, class C>
		static inline decltype(auto) install(Core &core, C&& c) {
			return Call<Plain<Args>...>{core.template add<Plain<Args>...>(rpc::forward<C>(c))};
		}
	};
}

template <
	class NameRegistry,
	class IoEngine,
	class StreamWriterFactory, 
	template<class> class Pointer,
	template<class, class> class Registry
>
class Endpoint: Core<StreamWriterFactory, Pointer, Registry>, NameRegistry, public IoEngine
{
	using Accessor = typename Endpoint::Core::Accessor;
	using CallId = typename Endpoint::Core::CallId;
	static constexpr CallId lookupId = 0, invalidId = -1u;

public:
	bool init()
	{
		return this->Endpoint::Core::template addCallAt<StreamReader<char, Accessor>, Call<CallId>>(lookupId, [this]
		(StreamReader<char, Accessor> name, Call<CallId> callback)
		{
			auto nameQuery = this->beginQuery();

			const auto length = name.size();
			auto reader = name.begin();

			bool ok = true;
			for(auto i = 0u; i < length; i++)
			{
				char c;
				if(!reader.read(c))
				{
					// TODO Log: internal error
					ok = false;
					break;
				}

				nameQuery << c;
			}

			if(ok)
			{
				CallId result = invalidId;
				if(!nameQuery.run(result))
				{
					// TODO Log: unknown method looked up
				}

				if(!call(callback, result))
				{
					// TODO Log: failed to reply
				}
			}

		});
	}

	// template<class... CallArgs, class... InvokeArgs>
	// inline auto call(const RpcCall<CallArgs...> &c, InvokeArgs&&... args)
	// {
	// 	auto argTuple = pet::Tuple<CallArgs...>::create(std::forward<InvokeArgs>(args)...);

	// 	return static_cast<Child*>(this)->write([&](CallItem* ci) {
	// 		rpcCore.makeCallItem(ci, c.id, std::move(argTuple));
	// 	});
	// }

	using Endpoint::Core::execute;

	template<class Ret> class LookupResult;

	template<class C>
	inline auto install(C&& c) 
	{
		auto &core = *((typename Endpoint::Core*)this);
		return detail::CallOperatorSignatureUtility<decltype(&C::operator())>::install(core, rpc::move(c));
	}

	template<class... NominalArgs, class... ActualArgs>
	inline bool call(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
		bool buildOk;
		auto data = this->Endpoint::Core::template buildCall<NominalArgs...>(buildOk, call.id, rpc::forward<ActualArgs>(args)...);	
		if(!buildOk)
		{
			// TODO Log: could not create message
			return false;
		}

		bool sendOk = IoEngine::send(rpc::move(data));
		if(!sendOk)
		{
			// TODO Log: could not send message
			return false;
		}

		return true;
	}

#if 0
	template<class N, class C>
	inline constexpr bool provide(const N& name, C&& c)
	{
		auto sgn = detail::FullSignatureGenerator<C::operator()>::generateSignature(name);
		return addMapping((const char*)sgn, c.id);
	}

	template<class NominalArgs..., class... ActualArgs>
	inline constexpr bool call(Call<NominalArgs...> call, ActualArgs&&...)
	{
		
	}
#endif

	// template<class... Args>
	// inline auto install(pet::Delegate<void(Args...)>&& d) {
	// 	return RpcCall<Args...>{rpcCore.registerCall(std::move(d))};
	// }

	// template<class... Args>
	// inline void uninstall(const RpcCall<Args...> &c)
	// {
	// 	removeMapping(c.id);
	// 	auto callRemoved = rpcCore.removeCall(c.id);
	// 	assert(callRemoved);
	// }

private:
	// inline bool addMapping(std::string str, uint32_t id)
	// {
	// 	if(bwdLookupTable.find(id) != bwdLookupTable.end() || fwdLookupTable.find(str) != fwdLookupTable.end())
	// 		return false;

	// 	fwdLookupTable.insert(std::make_pair(str, id));
	// 	bwdLookupTable.insert(std::make_pair(id, str));
	// 	return true;
	// }

	// inline bool removeMapping(uint32_t id)
	// {
	// 	auto bwdIt = bwdLookupTable.find(id);

	// 	if(bwdIt == bwdLookupTable.end())
	// 		return false;

	// 	auto fwdIt = fwdLookupTable.find(bwdIt->second);
	// 	assert(fwdIt != fwdLookupTable.end());

	// 	fwdLookupTable.erase(fwdIt);
	// 	bwdLookupTable.erase(bwdIt);
	// 	return true;
	// }
};

// template<class Child>
// template<class... Args>
// class RpcProtocol<Child>::LookupResult<Call<Args...>>
// {
// 	RpcCall<uint32_t> cb;
// 	uint32_t ret;
// 	bool done = false;

// public:
// 	auto wait(RpcProtocol &prot, uint32_t maxWaitMs = 0) {
// 		return prot.processOne(maxWaitMs);
// 	}

// 	bool takeResult(Endpoint &prot, RpcCall<Args...>& out)
// 	{
// 		if(done)
// 		{
// 			if(cb.id != -1u)
// 			{
// 				prot.uninstall(cb);
// 				cb.id = -1u;
// 			}

// 			out.id = ret;
// 			return true;
// 		}

// 		return false;
// 	}

// 	inline auto lookup(Endpoint &prot, const char* name)
// 	{
// 		auto sgn = RpcSignatureGenerator<Args...>::generateSignature(name);

// 		assert(cb.id == -1);

// 		cb = prot.install(pet::delegate([this](uint32_t ret)
// 		{
// 			this->ret = ret;
// 			done = true;
// 		}));

// 		return prot.call(RpcCall<RpcCall<uint32_t>, const char*>{lookupId}, cb, sgn.c_str());
// 	}

// 	~LookupResult() {
// 		assert(cb.id == -1u);
// 	}
// };

}

#endif /* _RPCENDPOINT_H_ */

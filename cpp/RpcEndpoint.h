#ifndef _RPCENDPOINT_H_
#define _RPCENDPOINT_H_

#include "RpcCore.h"
#include "RpcCTStr.h"
#include "RpcSymbol.h"
#include "RpcArrayWriter.h"
#include "RpcStreamReader.h"
#include "RpcSignatureGenerator.h"

#include <unordered_map>

namespace rpc {

namespace detail 
{
	template<class> struct CallOperatorSignatureUtility;
	template<class T> using Plain = remove_const_t<remove_reference_t<T>>;

	template<class Type, class... Args> struct CallOperatorSignatureUtility<void (Type::*)(Args...) const>
	{
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

	bool doLookup(const char* name, size_t length, CallId cb)
	{
		bool buildOk;

		auto data = this->Endpoint::Core::template buildCall<ArrayWriter<char>, Call<CallId>>(
			buildOk, lookupId, ArrayWriter<char>(name, length), Call<CallId>{cb});	

		if(buildOk)
		{
			if(IoEngine::send(rpc::move(data)))
			{
				return true;
			}
			else
			{
				// TODO Log: could not create lookup message
			}
		}
		else
		{
			// TODO Log: could not send message
		}

		return false;
	}

public:
	bool init()
	{
		return this->Endpoint::Core::template addCallAt<StreamReader<char, Accessor>, Call<CallId>>(lookupId, [this]
		(StreamReader<char, Accessor> name, Call<CallId> callback)
		{
			auto nameQuery = this->NameRegistry::beginQuery();

			for(char c: name)
				nameQuery << c;

			CallId result = invalidId;
			if(!nameQuery.run(result))
			{
				// TODO Log: unknown method looked up
			}

			if(!call(callback, result))
			{
				// TODO Log: failed to reply
			}
		});
	}

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

	template<size_t n, class... Args, class C>
	inline auto provide(Symbol<n, Args...> sym, C&& c) 
	{
		auto &core = *((typename Endpoint::Core*)this);
		auto id = core.template add<detail::Plain<Args>...>(rpc::forward<C>(c));
		
		auto &nameReg = *((NameRegistry*)this);
		if(nameReg.addMapping((const char*)sym, id))
			return true;
		
		// TODO uninstall
		return false;
	}

	template<size_t n, class... Args, class C>
	inline auto lookup(const Symbol<n, Args...> &sym, C&& c) 
	{
		auto &core = *((typename Endpoint::Core*)this);
		auto id = core.template add<CallId>([this, c{rpc::forward<C>(c)}](CallId id)
		{
			c(Call<Args...>{id});
			// TODO uninstall
		});

		if(!doLookup((const char*)sym, n, id))
		{
			// TODO uninstall
			return false;
		}
		
		return true;
	}

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

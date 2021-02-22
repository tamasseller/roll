#ifndef _RPCENDPOINT_H_
#define _RPCENDPOINT_H_

#include "RpcCore.h"
#include "RpcStreamReader.h"
#include "RpcSignatureGenerator.h"

#include <unordered_map>

namespace rpc {

template <
	class NameRegistry,
	class StreamWriterFactory, 
	template<class> class Pointer,
	template<class, class> class Registry
>
class Endpoint: Core<StreamWriterFactory, Pointer, Registry>, NameRegistry
{
	static constexpr uint32_t lookupId = 0;

public:
	bool init()
	{
		this->addCallAt(lookupId, [this](StreamReader<StreamReader<char>> names, Call<StreamWriter<>>)
		{

		});
	}

	template<class... CallArgs, class... InvokeArgs>
	inline auto call(const RpcCall<CallArgs...> &c, InvokeArgs&&... args)
	{
		auto argTuple = pet::Tuple<CallArgs...>::create(std::forward<InvokeArgs>(args)...);

		return static_cast<Child*>(this)->write([&](CallItem* ci) {
			rpcCore.makeCallItem(ci, c.id, std::move(argTuple));
		});
	}

	inline auto processOne(uint32_t maxWaitMs = 0)
	{
		return static_cast<Child*>(this)->read(maxWaitMs, [this](CallItem* ci){
			if(!this->rpcCore.execute(ci))
				static_cast<Child*>(this)->handleUnknownCall();
		});
	}

	template<class Ret> class LookupResult;

	template<class... Args>
	inline bool provide(const char* name, const RpcCall<Args...> &c)
	{
		auto sgn = RpcSignatureGenerator<Args...>::generateSignature(name);
		return addMapping(sgn, c.id);
	}

	template<class... Args>
	inline auto install(pet::Delegate<void(Args...)>&& d) {
		return RpcCall<Args...>{rpcCore.registerCall(std::move(d))};
	}

	template<class... Args>
	inline void uninstall(const RpcCall<Args...> &c)
	{
		removeMapping(c.id);
		auto callRemoved = rpcCore.removeCall(c.id);
		assert(callRemoved);
	}

private:
	inline bool addMapping(std::string str, uint32_t id)
	{
		if(bwdLookupTable.find(id) != bwdLookupTable.end() || fwdLookupTable.find(str) != fwdLookupTable.end())
			return false;

		fwdLookupTable.insert(std::make_pair(str, id));
		bwdLookupTable.insert(std::make_pair(id, str));
		return true;
	}

	inline bool removeMapping(uint32_t id)
	{
		auto bwdIt = bwdLookupTable.find(id);

		if(bwdIt == bwdLookupTable.end())
			return false;

		auto fwdIt = fwdLookupTable.find(bwdIt->second);
		assert(fwdIt != fwdLookupTable.end());

		fwdLookupTable.erase(fwdIt);
		bwdLookupTable.erase(bwdIt);
		return true;
	}
};

template<class Child>
template<class... Args>
class RpcProtocol<Child>::LookupResult<Call<Args...>>
{
	RpcCall<uint32_t> cb;
	uint32_t ret;
	bool done = false;

public:
	auto wait(RpcProtocol &prot, uint32_t maxWaitMs = 0) {
		return prot.processOne(maxWaitMs);
	}

	bool takeResult(Endpoint &prot, RpcCall<Args...>& out)
	{
		if(done)
		{
			if(cb.id != -1u)
			{
				prot.uninstall(cb);
				cb.id = -1u;
			}

			out.id = ret;
			return true;
		}

		return false;
	}

	inline auto lookup(Endpoint &prot, const char* name)
	{
		auto sgn = RpcSignatureGenerator<Args...>::generateSignature(name);

		assert(cb.id == -1);

		cb = prot.install(pet::delegate([this](uint32_t ret)
		{
			this->ret = ret;
			done = true;
		}));

		return prot.call(RpcCall<RpcCall<uint32_t>, const char*>{lookupId}, cb, sgn.c_str());
	}

	~LookupResult() {
		assert(cb.id == -1u);
	}
};

}

#endif /* _RPCENDPOINT_H_ */

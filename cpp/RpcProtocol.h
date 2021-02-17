#ifndef COMMON_RPCPROTOCOL_H_
#define COMMON_RPCPROTOCOL_H_

#include "RpcCore.h"
#include "RpcSignatureGenerator.h"
#include "RpcSerdes.h"
#include "Common.h"
#include "Log.h"

template<class Child>
class RpcProtocol
{
	static constexpr uint32_t lookupId = 0;

	RpcCore<RpcSerializer, RpcDeserializer> rpcCore;
	std::unordered_map<std::string, uint32_t> fwdLookupTable;
	std::unordered_map<uint32_t, std::string> bwdLookupTable;

public:
	using ListCb = RpcCall<const char*>;
	using ListFwd = RpcCall<ListCb>;
	static constexpr const char *listName = "lsrpc";

	inline RpcProtocol()
	{
		assert(rpcCore.registerCall(lookupId, pet::delegate([this](RpcCall<uint32_t> cb, const char* name)
		{
			auto it = fwdLookupTable.find(name);

			if(it == fwdLookupTable.end())
				Log(Log::Level::Warning) << "serving RPC name lookup for unknown '" << name << "'";

			call(cb, it != fwdLookupTable.end() ? it->second : -1u);
		})));

		ListFwd listFwd = install(pet::delegate([this](ListCb cb)
		{
			for(auto& x: fwdLookupTable)
				call(cb, x.first.c_str());

			call(cb, "");
		}));

		bool lsProvideOk = provide(listName, listFwd);
		assert(lsProvideOk);
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
class RpcProtocol<Child>::LookupResult<RpcCall<Args...>>
{
	RpcCall<uint32_t> cb;
	uint32_t ret;
	bool done = false;

public:
	auto wait(RpcProtocol &prot, uint32_t maxWaitMs = 0) {
		return prot.processOne(maxWaitMs);
	}

	bool takeResult(RpcProtocol &prot, RpcCall<Args...>& out)
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

	inline auto lookup(RpcProtocol &prot, const char* name)
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

#endif /* COMMON_RPCPROTOCOL_H_ */

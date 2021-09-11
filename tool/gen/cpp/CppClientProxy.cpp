#include "CppClientProxy.h"

#include "CppCommon.h"
#include "CppProxyCommon.h"

#include <algorithm>

struct SymbolReferenceExtractor
{
	struct SymRef {
		std::string type, name, symbol;
	};

	static inline void handleItem(std::vector<SymRef> &ret, const Contract::Function &f, const std::string& t, const std::string& s) {
		ret.push_back({t + "::" + ((f.returnType) ? functionSignatureTypeName(f.name) : actionSignatureTypeName(f.name)), f.name, t + "::" + symbolName(f.name)});
	}

	static inline void handleSessionItem(std::vector<SymRef> &ret, const Contract::Session::Ctor & c, const std::string& ct, const std::string& cs, const std::string& sName)
	{
		const auto type = ct + "::" + sessionNamespaceName(sName) + "::" + sessionCreateSignatureTypeName(c.name);
		const auto name = sessionCtorApiName(sName, c.name);
		const auto symbol = cs + "::" + sessionNamespaceName(sName) + "::" + symbolName(c.name);
		ret.push_back({type, name, symbol});
	}

	template<class C> static inline void handleSessionItem(std::vector<SymRef> &ret, const C&, const std::string&, const std::string&, const std::string&) {}

	static inline void handleItem(std::vector<SymRef> &ret, const Contract::Session &s, const std::string& ct, const std::string& cs)
	{
		for(const auto& i: s.items) {
			std::visit([&ret, &ct, &cs, sName{s.name}](const auto& i){ return handleSessionItem(ret, i, ct, cs, sName); }, i.second);
		}
	}

	template<class C> static inline void handleItem(std::vector<SymRef> &ret, const C&, const std::string&, const std::string&) {}

	static inline auto gatherSymbolReferences(const Contract& c)
	{
		std::vector<SymRef> ret;

		const auto t = contractTypesNamespaceName(c.name);
		const auto s = contractSymbolsNamespaceName(c.name);

		for(const auto& i: c.items) {
			std::visit([&ret, &t, &s](const auto i){handleItem(ret, i, t, s);}, i.second);
		}

		return ret;
	}
};

void writeClientProxy(std::stringstream& ss, const Contract& c)
{
	const auto symbolRefences = SymbolReferenceExtractor::gatherSymbolReferences(c);

	std::vector<std::string> strs;

	std::transform(symbolRefences.begin(), symbolRefences.end(), std::back_inserter(strs), [](const auto& it)
	{
		return indent(1) + it.type + " " + callMemberName(it.name) + ";";
	});

	writeProxy(ss, clientProxyName(c.name), strs);
}

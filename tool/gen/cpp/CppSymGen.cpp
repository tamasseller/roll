#include "CppSymGen.h"

#include "CppCommon.h"

#include <algorithm>

struct CommonSymbolGenerator
{
	template<class C>
	static inline std::string handleItem(const std::string& cName, const C &f, const int n) { return {}; }

	static inline std::string symbol(const std::string& name, const std::string& type, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "static constexpr inline auto " << symbolName(name);
		ss << " = rpc::symbol(" << type << "(), \"" << name << "\"_ctstr);";
		return ss.str();
	}

	static inline std::string handleItem(const std::string& cName, const Contract::Function &f, const int n)
	{
		return symbol(f.name, cName + "::" + ((f.returnType) ? functionSignatureTypeName(f.name) : actionSignatureTypeName(f.name)), n);
	}

	static inline std::string handleSessionItem(const std::string& typeName, const Contract::Session::Ctor & c, const int n) {
		return symbol(c.name, typeName + "::" + sessionCreateSignatureTypeName(c.name) , n);
	}

	template<class C> static inline std::string handleSessionItem(const std::string&, const C&, const int n) { return {}; }

	static inline std::string handleItem(const std::string& cName, const Contract::Session &s, const int n)
	{
		std::stringstream ss;

		std::vector<std::string> strs;
		std::transform(s.items.begin(), s.items.end(), std::back_inserter(strs), [n, t{cName + "::" + sessionNamespaceName(s.name)}](const auto &it){
			return std::visit([n, t](const auto& i){ return handleSessionItem(t, i, n + 1); }, it.second);
		});

		writeBlock(ss, "struct " + sessionNamespaceName(s.name), strs, n);
		ss << ";";
		return ss.str();
	}
};

void writeContractSymbols(std::stringstream &ss, const Contract& c)
{
	std::vector<std::string> strs;

	std::transform(c.items.begin(), c.items.end(), std::back_inserter(strs), [&c, t{contractTypeBlockNameRef(c.name)}](const auto &i){
		return std::visit([&c, t](const auto& i){ return CommonSymbolGenerator::handleItem(t, i, 1); }, i.second);
	});

	writeTopLevelBlock(ss, "struct " + contractSymbolsBlockNameRef(c.name), strs);
}

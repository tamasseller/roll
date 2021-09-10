#include "CppTypeGen.h"

#include "CppCommon.h"

#include <algorithm>
#include <list>
#include <iterator>

void writeNamespace(std::stringstream& ss, const std::string name, const std::vector<std::string>& strs, const int n)
{
	bool first = true;
	bool prevMultiLine;
	for(const auto& s: strs)
	{
		if(s.length())
		{
			bool multiline = s.find_first_of('\n') != std::string::npos;

			if(first)
			{
				first = false;
				ss << indent(n) << "namespace " << name << std::endl;
				ss << indent(n) << "{" << std::endl;
			}
			else
			{
				if(multiline || prevMultiLine)
				{
					ss << std::endl;
				}
			}

			ss << s << std::endl;
			prevMultiLine = multiline;
		}
	}

	if(!first)
	{
		ss << indent(n) << "};";
	}
}

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

		writeNamespace(ss, sessionNamespaceName(s.name), strs, n);
		return ss.str();
	}
};

void writeContractSymbols(std::stringstream &ss, const Contract& c)
{
	std::vector<std::string> strs;

	std::transform(c.items.begin(), c.items.end(), std::back_inserter(strs), [&c, t{contractTypesNamespaceName(c.name)}](const auto &i){
		return std::visit([&c, t](const auto& i){ return CommonSymbolGenerator::handleItem(t, i, 1); }, i.second);
	});

	writeNamespace(ss, contractSymbolsNamespaceName(c.name), strs, 0);
	ss << std::endl << std::endl;
}

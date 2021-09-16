#include "CppStructSerdes.h"

#include "CppCommon.h"

#include <algorithm>

struct StructTypeInfoGenerator
{
	static inline std::string serDesEntry(const std::string& name, const std::vector<std::string>& a, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "template<> struct TypeInfo<" << name << ">: StructTypeInfo<" << name;

		if(a.size() > 1)
		{
			for(const auto& m: a)
			{
				ss << "," << std::endl << indent(n + 1) << "StructMember<&" << name << "::" << m << ">";
			}

			ss << std::endl << indent(n) << "> {};";
		}
		else
		{
			ss << ", " << "StructMember<&" << name << "::" << a.front() << ">> {};";
		}

		return ss.str();
	}

	static inline std::string handleTypeDef(const std::string& name, const Contract::Aggregate& a, const int n)
	{
		std::vector<std::string> contents;
		std::transform(a.members.begin(), a.members.end(), std::back_inserter(contents), [](const auto &i){ return i.name; });
		return serDesEntry(name, contents, n);
	}

	template<class T>
	static inline std::string handleTypeDef(const std::string& name, const T& t, const int n) { return {}; }

	static inline std::string handleItem(const std::string& contractName, const Contract::Alias &a, const int n) {
		return std::visit([name{contractTypeBlockNameRef(contractName) + "::" + userTypeName(a.name)}, n](const auto &t){ return handleTypeDef(name, t, n); }, a.type);
	}

	template<class T> static inline void handleSessionItem(const T& t, std::vector<std::string> &fwd, std::vector<std::string> &bwd) {}

	static inline void handleSessionItem(const Contract::Session::ForwardCall &t, std::vector<std::string> &fwd, std::vector<std::string> &bwd) {
		fwd.push_back(t.name);
	}

	static inline void handleSessionItem(const Contract::Session::CallBack &t, std::vector<std::string> &fwd, std::vector<std::string> &bwd) {
		bwd.push_back(t.name);
	}

	static inline std::string handleItem(const std::string& contractName, const Contract::Session &s, const int n)
	{
		std::vector<std::string> fwd, bwd;

		for(const auto &i: s.items)
		{
			std::visit([&fwd, &bwd](const auto &t){ return handleSessionItem(t, fwd, bwd); }, i.second);
		}

		const auto baseName = contractTypeBlockNameRef(contractName) + "::" + sessionNamespaceName(s.name);
		std::stringstream ss;

		if(fwd.size())
		{
			ss << serDesEntry(baseName + "::" + sessionCallExportTypeName(s.name), fwd, n) << std::endl;
		}

		if(bwd.size())
		{
			if(fwd.size())
			{
				ss << std::endl;
			}

			ss << serDesEntry(baseName + "::" + sessionCallbackExportTypeName(s.name), bwd, n);
		}

		return ss.str();
	}

	template<class C> static inline std::string handleItem(const std::string&, const C&, const int n) { return {}; }
};

void writeStructTypeInfo(std::stringstream &ss, const Contract& c)
{
	std::vector<std::string> strs;

	std::transform(c.items.begin(), c.items.end(), std::back_inserter(strs), [&c](const auto &i){
		return std::visit([&c](const auto& i){ return StructTypeInfoGenerator::handleItem(c.name, i, 1); }, i.second);
	});

	writeTopLevelBlock(ss, "namespace rpc", strs, false);
}

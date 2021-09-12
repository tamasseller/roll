#include "CppStructSerdes.h"

#include "CppCommon.h"

#include <algorithm>

struct StructTypeInfoGenerator
{
	static inline std::string handleTypeDef(const std::string& name, const Contract::Aggregate& a, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "template<> struct TypeInfo<" << name << ">: StructTypeInfo<" << name;

		if(a.members.size() > 1)
		{
			for(const auto& m: a.members)
			{
				ss << "," << std::endl << indent(n + 1) << "StructMember<&" << name << "::" << m.name << ">";
			}

			ss << std::endl << indent(n) << "> {};";
		}
		else
		{
			ss << ", " << "StructMember<&" << name << "::" << a.members.front().name << ">> {};";
		}

		return ss.str();
	}

	template<class T>
	static inline std::string handleTypeDef(const std::string& name, const T& t, const int n) { return {}; }

	static inline std::string handleItem(const std::string& contractName, const Contract::Alias &a, const int n) {
		return std::visit([name{contractTypesNamespaceName(contractName) + "::" + userTypeName(a.name)}, n](const auto &t){ return handleTypeDef(name, t, n); }, a.type);
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

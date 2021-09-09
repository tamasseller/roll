#include "CppStructSerdes.h"

#include "CppCommon.h"

struct StructTypeInfoGenerator
{
	static inline std::string handleTypeDef(const std::string& name, const Contract::Aggregate& a, const int n)
	{
		std::stringstream ss;
		ss << indent(n) << "template<> struct TypeInfo<" << name << ">: StructTypeInfo<" << name;

		for(const auto& m: a.members)
		{
			ss << "," << std::endl << indent(n + 1) << "StructMember<&" << name << "::" << m.name << ">";
		}

		ss << std::endl << indent(n) << ">;" << std::endl;
		return ss.str();
	}

	template<class T>
	static inline std::string handleTypeDef(const std::string& name, const T& t, const int n) { return {}; }

	static inline std::string handleItem(const std::string& contractName, const Contract::Alias &a, const int n) {
		return std::visit([name{contractTypeName(contractName) + "::" + userTypeName(a.name)}, n](const auto &t){ return handleTypeDef(name, t, n); }, a.type);
	}

	template<class C> static inline std::string handleItem(const std::string&, const C&, const int n) { return {}; }
};

void writeStructTypeInfo(std::stringstream &ss, const Contract& c)
{
	ss << "namespace rpc" << std::endl;
	ss << "{" << std::endl;

	bool first = true;
	for(const auto& i: c.items)
	{
		const auto str = std::visit([&c](const auto& i){ return StructTypeInfoGenerator::handleItem(c.name, i, 1); }, i.second);

		if(str.length())
		{
			if(first)
			{
				first = false;
			}
			else
			{
				ss << std::endl;
			}
		}

		ss << str;
	}

	ss << "};" << std::endl << std::endl;
}

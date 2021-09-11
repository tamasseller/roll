#include "CppProxyCommon.h"

void writeProxy(std::stringstream& ss, const std::string& name, const std::vector<std::string>& content)
{
	ss << "template<class Rpc>" << std::endl;
	ss << "class " << name << ": Rpc" << std::endl;
	ss << "{" << std::endl;

	for(const auto& c: content)
	{
		if(c.length())
		{
			ss << c << std::endl;
		}
	}

	ss << "};" << std::endl;
}

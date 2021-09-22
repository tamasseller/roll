#include "Cpp.h"
#include "CppSymGen.h"
#include "CppCommon.h"
#include "CppTypeGen.h"
#include "CppClientProxy.h"
#include "CppServerProxy.h"
#include "CppSessionProxy.h"
#include "CppStructSerdes.h"

const CodeGenCpp CodeGenCpp::instance;

static inline std::string allcapsEscape(const std::string &str)
{
	std::string ret(str.size(), '\0');

	for(auto i = 0u; i < str.size(); i++)
	{
		ret[i] = std::isalnum(str[i]) ? std::toupper(str[i]) : '_';
	}

	return ret;
}


std::string CodeGenCpp::generate(const std::vector<Contract>& cs, const std::string& name, bool doClient, bool doService) const
{
	std::stringstream ss;

	const auto guardMacroName = "_" + allcapsEscape(name) + "_";

	ss << "#ifndef " << guardMacroName << std::endl;
	ss << "#define " << guardMacroName << std::endl << std::endl;

	ss << "#include \"RpcCall.h\"" << std::endl;

	if(doClient)
	{
		ss << "#include \"RpcClient.h\"" << std::endl;
	}

	ss << "#include \"RpcStruct.h\"" << std::endl;
	ss << "#include \"RpcSymbol.h\"" << std::endl;
	ss << "#include \"RpcSession.h\"" << std::endl;

	if(doClient)
	{
		ss << "#include \"RpcService.h\"" << std::endl;
	}

	ss << "#include \"RpcTypeInfo.h\"" << std::endl << std::endl;

	for(const auto& c: cs)
	{
		writeTopLevelBlock(ss, "struct " + contractRootBlockName(c.name),
		{
			indent(1) + "class Types;",
			indent(1) + "class Symbols;",
			indent(1) + "template<class> class ClientProxy;",
			indent(1) + "template<class, class> class ServerProxy;"
		});

		writeContractTypes(ss, c);
		writeStructTypeInfo(ss, c);
		writeContractSymbols(ss, c);

		if(doClient)
		{
			writeSessionProxies(ss, c, ClientSessionProxyFilterFactory{});
			writeClientProxy(ss, c);
		}

		if(doService)
		{
			writeSessionProxies(ss, c, ServerSessionProxyFilterFactory{});
			writeServerProxy(ss, c);
		}
	}

	ss << std::endl << "#endif /* " << guardMacroName << " */" << std::endl;
	return ss.str();
}

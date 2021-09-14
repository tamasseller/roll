#include "Cpp.h"
#include "CppSymGen.h"
#include "CppTypeGen.h"
#include "CppClientProxy.h"
#include "CppServerProxy.h"
#include "CppStructSerdes.h"

const CodeGenCpp CodeGenCpp::instance;

std::string CodeGenCpp::generate(const std::vector<Contract>& cs, bool doClient, bool doService) const
{
	std::stringstream ss;
	ss << "#include \"RpcCall.h\"" << std::endl;
	ss << "#include \"RpcSymbol.h\"" << std::endl;
	ss << "#include \"RpcStruct.h\"" << std::endl;
	ss << "#include \"RpcTypeInfo.h\"" << std::endl;

	if(doClient)
	{
		ss << "#include \"RpcClient.h\"" << std::endl << std::endl;
	}

	for(const auto& c: cs)
	{
		writeContractTypes(ss, c);
		writeStructTypeInfo(ss, c);
		writeContractSymbols(ss, c);

		if(doClient)
		{
			writeClientProxy(ss, c);
		}

		if(doService)
		{
			writeServerProxy(ss, c);
		}
	}

	return ss.str();
}

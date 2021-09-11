#include "Cpp.h"
#include "CppSymGen.h"
#include "CppTypeGen.h"
#include "CppClientProxy.h"
#include "CppServerProxy.h"
#include "CppStructSerdes.h"

const CodeGenCpp CodeGenCpp::instance;

void writeCommonHeader(std::stringstream &ss)
{
	ss << "#include \"RpcCall.h\"" << std::endl;
	ss << "#include \"RpcSymbol.h\"" << std::endl;
	ss << "#include \"RpcTypeInfo.h\"" << std::endl;
	ss << "#include \"RpcStruct.h\"" << std::endl;
}

template<void (*directionSpecific)(std::stringstream&, const Contract&)>
void generateSource(std::stringstream &ss, const std::vector<Contract>& cs)
{
	for(const auto& c: cs)
	{
		writeContractTypes(ss, c);
		writeStructTypeInfo(ss, c);
		writeContractSymbols(ss, c);
		directionSpecific(ss, c);
	}
}

std::string CodeGenCpp::generateClient(const std::vector<Contract>& cs) const
{
	std::stringstream ss;
	writeCommonHeader(ss);
	ss << std::endl;
	generateSource<writeClientProxy>(ss, cs);
	ss << std::endl;
	return ss.str();
}

std::string CodeGenCpp::generateServer(const std::vector<Contract>& cs) const
{
	std::stringstream ss;
	writeCommonHeader(ss);
	ss << std::endl;
	generateSource<writeServerProxy>(ss, cs);
	ss << std::endl;
	return ss.str();
}

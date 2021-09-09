#include "Cpp.h"
#include "CppCommon.h"
#include "CppTypeGen.h"

const CodeGenCpp CodeGenCpp::instance;

void writeHeader(std::stringstream &ss)
{
	ss << "#include \"RpcCall.h\"" << std::endl;
	ss << "#include \"RpcSymbol.h\"" << std::endl;
	ss << "#include \"RpcTypeInfo.h\"" << std::endl;
	ss << std::endl;
}

std::string CodeGenCpp::generateClient(const std::vector<Contract>& cs) const
{
	std::stringstream ss;
	writeHeader(ss);

	for(const auto& c: cs)
	{
		writeContractTypes(ss, c);
	}

	return ss.str();
}

std::string CodeGenCpp::generateServer(const std::vector<Contract>& cs) const
{
	std::stringstream ss;
	writeHeader(ss);

	for(const auto& c: cs)
	{
		writeContractTypes(ss, c);
	}

	return ss.str();
}

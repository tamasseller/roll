#include "Cpp.h"
#include "CppSymGen.h"
#include "CppTypeGen.h"
#include "CppStructSerdes.h"

const CodeGenCpp CodeGenCpp::instance;

void writeServerProxy(std::istream&, const Contract&) {}
void writeClientProxy(std::istream&, const Contract&) {}

template<void (*directionSpecific)(std::istream&, const Contract&)>
std::string generateSource(const std::vector<Contract>& cs)
{
	std::stringstream ss;
	ss << "#include \"RpcCall.h\"" << std::endl;
	ss << "#include \"RpcSymbol.h\"" << std::endl;
	ss << "#include \"RpcTypeInfo.h\"" << std::endl;
	ss << "#include \"RpcStruct.h\"" << std::endl;
	ss << std::endl;

	for(const auto& c: cs)
	{
		writeContractTypes(ss, c);
		writeStructTypeInfo(ss, c);
		writeContractSymbols(ss, c);
		directionSpecific(ss, c);
	}

	return ss.str();
}

std::string CodeGenCpp::generateClient(const std::vector<Contract>& cs) const {
	return generateSource<writeClientProxy>(cs);
}

std::string CodeGenCpp::generateServer(const std::vector<Contract>& cs) const {
	return generateSource<writeServerProxy>(cs);
}

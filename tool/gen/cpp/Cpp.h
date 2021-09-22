#ifndef RPC_TOOL_GEN_CPP_H_
#define RPC_TOOL_GEN_CPP_H_

#include "../Generator.h"

class CodeGenCpp: public CodeGen
{
	inline virtual ~CodeGenCpp() = default;
	virtual std::string generate(const std::vector<Contract>& contract, const std::string& name, bool doClient, bool doService) const override;

public:
	static const CodeGenCpp instance;
};

#endif /* RPC_TOOL_GEN_CPP_H_ */

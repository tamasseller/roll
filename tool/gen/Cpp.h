#ifndef RPC_TOOL_GEN_CPP_H_
#define RPC_TOOL_GEN_CPP_H_

#include "Generator.h"

class CodeGenCpp: public CodeGen
{
	inline virtual ~CodeGenCpp() = default;

	virtual std::string generateClient(const Contract&) const override;
	virtual std::string generateServer(const Contract&) const override;

public:
	static const CodeGenCpp instance;
};

#endif /* RPC_TOOL_GEN_CPP_H_ */

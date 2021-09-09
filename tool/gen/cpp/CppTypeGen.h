#ifndef RPC_TOOL_GEN_CPP_CPPTYPEGEN_H_
#define RPC_TOOL_GEN_CPP_CPPTYPEGEN_H_

#include "ast/Contract.h"

#include <sstream>

void writeContractTypes(std::stringstream &ss, const Contract& c);

#endif /* RPC_TOOL_GEN_CPP_CPPTYPEGEN_H_ */

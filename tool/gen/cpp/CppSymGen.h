#ifndef RPC_TOOL_GEN_CPP_CPPSYMGEN_H_
#define RPC_TOOL_GEN_CPP_CPPSYMGEN_H_

#include "ast/Contract.h"

#include <sstream>

void writeContractSymbols(std::stringstream &ss, const Contract& c);

#endif /* RPC_TOOL_GEN_CPP_CPPSYMGEN_H_ */

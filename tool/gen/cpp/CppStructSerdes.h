#ifndef RPC_TOOL_GEN_CPP_CPPSTRUCTSERDES_H_
#define RPC_TOOL_GEN_CPP_CPPSTRUCTSERDES_H_

#include "ast/Contract.h"

#include <sstream>

void writeStructTypeInfo(std::stringstream &ss, const Contract& c);

#endif /* RPC_TOOL_GEN_CPP_CPPSTRUCTSERDES_H_ */

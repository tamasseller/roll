#ifndef RPC_TOOL_GEN_CPP_CPPSERVERPROXY_H_
#define RPC_TOOL_GEN_CPP_CPPSERVERPROXY_H_

#include "ast/Contract.h"

#include <sstream>

void writeServerProxy(std::stringstream&, const Contract&);

#endif /* RPC_TOOL_GEN_CPP_CPPSERVERPROXY_H_ */

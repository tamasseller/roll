#ifndef RPC_TOOL_GEN_CPP_CPPPROXYCOMMON_H_
#define RPC_TOOL_GEN_CPP_CPPPROXYCOMMON_H_

#include <vector>
#include <string>
#include <sstream>

void writeProxy(std::stringstream& ss, const std::string& name, const std::vector<std::string>& content);

#endif /* RPC_TOOL_GEN_CPP_CPPPROXYCOMMON_H_ */

#ifndef RPC_TOOL_ASTPARSER_H_
#define RPC_TOOL_ASTPARSER_H_

#include "Contract.h"
#include <iosfwd>

std::vector<Contract> parse(std::istream& is);

#endif /* RPC_TOOL_ASTPARSER_H_ */

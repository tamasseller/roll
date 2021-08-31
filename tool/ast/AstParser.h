#ifndef RPC_TOOL_ASTPARSER_H_
#define RPC_TOOL_ASTPARSER_H_

#include "Ast.h"
#include <iosfwd>

Ast parse(std::istream& is);

#endif /* RPC_TOOL_ASTPARSER_H_ */

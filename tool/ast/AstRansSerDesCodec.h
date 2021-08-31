#ifndef RPC_TOOL_ASTRANSMODEL_H_
#define RPC_TOOL_ASTRANSMODEL_H_

#include "Ast.h"
#include <iosfwd>
#include <string>

Ast deserialize(std::istream &input);
std::string serialize(const Ast& ast);

#endif /* RPC_TOOL_ASTRANSMODEL_H_ */

#ifndef RPC_TOOL_DUMPMODE_H_
#define RPC_TOOL_DUMPMODE_H_

#include "Mode.h"

struct DumpMode: Mode<DumpMode>
{
	virtual int run(Ast ast, std::vector<std::string> args) const override;
};

#endif /* RPC_TOOL_DUMPMODE_H_ */

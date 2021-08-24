#ifndef RPC_TOOL_MODE_H_
#define RPC_TOOL_MODE_H_

#include "Ast.h"

struct ModeInterface
{
	inline virtual ~ModeInterface() = default;
	virtual int run(Ast ast, std::vector<std::string> args) const = 0;
};

template<class Child>
struct Mode: ModeInterface {
	static inline const Child instance;
};

#endif /* RPC_TOOL_MODE_H_ */

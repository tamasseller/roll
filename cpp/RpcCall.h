#ifndef COMMON_RPCCALL_H_
#define COMMON_RPCCALL_H_

#include <stdint.h>

template<class... Args>
struct RpcCall {
	uint32_t id = -1u;
};

#endif /* COMMON_RPCCALL_H_ */

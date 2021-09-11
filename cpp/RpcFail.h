#ifndef RPC_CPP_INTEROP_RPCFAIL_H_
#define RPC_CPP_INTEROP_RPCFAIL_H_

namespace rpc {

#ifdef __EXCEPTIONS
/**
 * RPC specific exception object.
 */

#include <exception>

class RpcException: public std::exception
{
    std::string str;

    virtual const char* what() const noexcept override {
        return str.c_str();
    }

public:
    inline RpcException(const std::string& str): str(std::string("RPC error: ") + str) {}
};

static inline void fail(const std::string& str) {
	throw RpcException(str);
}

#else

#include <iostream>

#include <stdlib.h>

static inline void fail(const std::string& str)
{
	std::cerr << str << std::endl;
	abort();
}

#endif

}

#endif /* RPC_CPP_INTEROP_RPCFAIL_H_ */

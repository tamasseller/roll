#ifndef RPC_CPP_INTEROP_RPCFAIL_H_
#define RPC_CPP_INTEROP_RPCFAIL_H_


#ifdef __EXCEPTIONS
/**
 * RPC specific exception object.
 */

#include <sstream>
#include <exception>

namespace rpc {

class RpcException: public std::exception
{
    std::string str;

    virtual const char* what() const noexcept override {
        return str.c_str();
    }

public:
    inline RpcException(const std::string& str): str(std::string("RPC error: ") + str) {}
};

template<class... C>
static inline void fail(C... args)
{
	const std::string strArgs[] = {args...};

	std::stringstream ss;
	for(const auto& s: strArgs)
	{
		ss << s;
	}

	throw RpcException(ss.str());
}

}


#else

#include <iostream>
#include <stdlib.h>

namespace rpc {

static inline void fail(const std::string& str)
{
	std::cerr << str << std::endl;
	abort();
}

}

#endif


#endif /* RPC_CPP_INTEROP_RPCFAIL_H_ */

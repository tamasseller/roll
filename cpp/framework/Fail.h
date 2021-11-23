#ifndef RPC_CPP_INTEROP_RPCFAIL_H_
#define RPC_CPP_INTEROP_RPCFAIL_H_

/*
 * Platform dependent failure handling routine definition.
 *
 * Its purpose is to adapt to the optional usage of exception handling.
 */

#ifdef __EXCEPTIONS

#include <sstream>
#include <exception>

namespace rpc {

/**
 * RPC specific exception object.
 */
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

namespace rpc {

void fatalRpcError(const char* diag);

template<class... C>
static inline void fail(const C&... strs)
{
	char unsafeErrorBuffer[128];

	const char* a[] = {strs...};
	auto ptr = unsafeErrorBuffer;
	const auto end = ptr + sizeof(unsafeErrorBuffer) - 1;

	for(const char* s: a)
	{
		while(char c = *s++)
		{
			*ptr++ = c;
			if(ptr == end)
			{
				break;
			}
		}

		if(ptr == end)
		{
			break;
		}
	}

	*ptr = 0;
	fatalRpcError(unsafeErrorBuffer);
}

}

#endif


#endif /* RPC_CPP_INTEROP_RPCFAIL_H_ */

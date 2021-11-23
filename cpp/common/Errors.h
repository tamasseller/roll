#ifndef _RPCERRORS_H_
#define _RPCERRORS_H_

#define ROLL_ERROR_CODE_LIST()                                                    \
	X(success,                     nullptr,                                       None) \
    X(internalError,               "internal error occurred",                     Die)  \
    X(symbolAlreadyExported,       "symbol already exported",                     Die)  \
    X(symbolNotFound,              "there is no such symbol registered",          Die)  \
    X(methodNotFound,              "there is no such method registered",          Die)  \
	X(couldNotSendMessage,         "could not send message",                      Drop) \
    X(couldNotCreateMessage,       "could not create message",                    Drop) \
    X(couldNotSendLookupMessage,   "unable to send lookup message",               Drop) \
    X(couldNotCreateLookupMessage, "unable to create lookup message",             Drop) \
    X(messageFormatError,          "message format error",                        Drop) \
	X(undefinedMethodCalled,       "the peer tried to invoke an unknown method",  Drop) \
	X(unknownSymbolRequested,      "the peer looked up an unknown symbol",        Log)

namespace rpc
{
	enum class ExpectedExecutorBehavior { None, Log, Drop, Die };

    /**
     * All possible error values used by the library.
     */
    enum class Errors
    {
#define X(x, y, ...) x,
    	ROLL_ERROR_CODE_LIST()
#undef X
    	unknownError
    };

    static constexpr inline bool operator!(Errors e) {
      return e == Errors::success;
    }

    /**
     * Get textual description of an error value.
     */
    static constexpr const char* getErrorString(Errors r)
    {
    	switch(r)
    	{
#define X(x, y, ...) case Errors:: x: return y;
    	ROLL_ERROR_CODE_LIST()
#undef X
    	default: return "!!! UNKNOWN ERROR !!!";
    	}
    }

    /**
     * Get associated expected behavior of the remote message processor loop.
     */
    static constexpr ExpectedExecutorBehavior getExpectedExecutorBehavior(Errors r)
    {
    	switch(r)
    	{
#define X(x, _, y) case Errors:: x: return ExpectedExecutorBehavior:: y;
    	ROLL_ERROR_CODE_LIST()
#undef X
    	default: return ExpectedExecutorBehavior::Die;
    	}
    }
}

#endif /* _RPCERRORS_H_ */

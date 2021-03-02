#ifndef _RPCERRORS_H_
#define _RPCERRORS_H_

namespace rpc
{
    /**
     * All possible error strings logged by the RPC engine.
     */
    struct Errors
    {
        static constexpr const char *internalError = "internal error occured";

        static constexpr const char *unknownMethodRequested = "unknown method looked up";

        static constexpr const char *couldNotSendMessage = "could not send message";
        static constexpr const char *couldNotCreateMessage = "could not create message";

        static constexpr const char *couldNotSendLookupMessage = "unable to send lookup message";
        static constexpr const char *couldNotCreateLookupMessage = "unable to create lookup message";

        static constexpr const char *symbolAlreadyExported = "symbol already exported";
        static constexpr const char *noSuchSymbol = "np such symbol";

        static constexpr const char *methodNotFound = "method not found";

        static constexpr const char *wrongMethodRequest = "wrong method request";
        static constexpr const char *messageFormatError = "message format error";
    };
}

#endif /* _RPCERRORS_H_ */

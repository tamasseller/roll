namespace rpc
{
    /**
     * All possible error strings logged by the RPC engine.
     */
    struct Errors
    {
        static constexpr const char *internalError = "internal error occured";

        static constexpr const char *unknownMethodRequested = "unknown method looked up";
        static constexpr const char *failedToReplyToLookup = "failed to reply to lookup";

        static constexpr const char *couldNotSendMessage = "could not send message";
        static constexpr const char *couldNotCreateMessage = "could not create message";

        static constexpr const char *couldNotSendLookupMessage = "could not send lookup message";
        static constexpr const char *couldNotCreateLookupMessage = "could not create lookup message";
    };
}
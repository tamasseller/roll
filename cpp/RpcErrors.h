namespace rpc
{
    /**
     * All possible error strings logged by the RPC engine.
     */
    struct Errors
    {
        static constexpr const char internalError[] = "internal error occured";

        static constexpr const char unknownMethodLookedUp[] = "unknown method looked up";
        static constexpr const char failedToReplyToValidLookup[] = "failed to reply to valid lookup";

        static constexpr const char couldNotSendUserMessage[] = "could not send user message";
        static constexpr const char couldNotCreateUserMessage[] = "could not create user message";

        static constexpr const char couldNotSendLookupMessage[] = "could not send lookup message";
        static constexpr const char couldNotCreateLookupMessage[] = "could not create lookup message";
    };
}
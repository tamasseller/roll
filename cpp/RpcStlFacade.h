#ifndef _RPCSTLFACADE_H_
#define _RPCSTLFACADE_H_

#include "RpcEndpoint.h"
#include "RpcStlAdapters.h"

#include <future>
#include <string.h>

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

/**
 * Facade for the generic RPC endpoint using STL based implementation of auxiliary operations.
 * 
 * This is supposed to be the regular frontend class for PC-like environments - i.e. wherever 
 * using the STL classes is advisable. This also means that dynamic memory usage is managed by
 * the STL implementation. When tighter control ver heap usage is a requirement alternate
 * implementations for the dependencies can be used.
 */
template<class IoEngine>
class StlFacade: Endpoint<
    detail::StlAutoPointer, 
    detail::HashMapRegistry,
    detail::HashMapBasedNameDictionary, 
    IoEngine
>{
	friend IoEngine;

public:
	using StlFacade::Endpoint::init;
    using StlFacade::Endpoint::process;
    using StlFacade::Endpoint::install;
    using StlFacade::Endpoint::uninstall;

    /**
     * The type of the 
     * 
     * Needed due to private baseclass. 
     */
    using Endpoint = typename StlFacade::Endpoint;

    /**
     * Accessor to the underlying IoEngine object.
     * 
     * Needed due to private baseclass. 
     */
    inline constexpr operator IoEngine*() { return (IoEngine*)this; }

    /**
     * Wrapper around Endpoint::init.
     */
    template<class... Args>
    StlFacade(Args&&... args): StlFacade::Endpoint(std::forward<Args>(args)...)
    {
        if(!this->init())
            throw RpcException("failed to initialize");
    }

    /**
     * Wrapper around Endpoint::provide
     * 
     * Throws instead of returning error.
     */
    template<size_t n, class... Args, class C>
	inline void provide(Symbol<n, Args...> sym, C&& c) 
	{
        if(auto err = this->StlFacade::Endpoint::provide(sym, std::forward<C>(c)))
            throw RpcException(err);
    }

    /**
     * Wrapper around Endpoint::discard
     * 
     * Throws instead of returning error.
     */
    template<size_t n, class... Args>
	inline void discard(Symbol<n, Args...> sym) 
	{
        if(auto err = this->StlFacade::Endpoint::discard(sym))
            throw RpcException(err);
    }

    /**
     * Wrapper around Endpoint::lookup
     * 
     * Throws instead of returning error.
     */
    template<size_t n, class... Args>
	inline std::future<rpc::Call<Args...>> lookup(const Symbol<n, Args...> &sym) 
	{
		std::promise<rpc::Call<Args...>> p;
        auto ret = p.get_future();

        const char* err = this->StlFacade::Endpoint::lookup(sym, [p{std::move(p)}](auto&, bool done, auto result) mutable 
        {
            if(done)
                p.set_value(result);
            else
                p.set_exception(std::make_exception_ptr(RpcException(Errors::noSuchSymbol)));
        });

		if(err)
            throw RpcException("TBD");

        return ret;
	}

    /**
     * Wrapper for Endpoint::call, no-callback case.
     * 
     * Throws instead of returning error.
     */
    template<class... NominalArgs, class... ActualArgs>
    inline void
	simpleCall(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
        if(auto err = this->StlFacade::Endpoint::call(call, std::forward<ActualArgs>(args)...))
            throw RpcException(err);
    }

    template<class... NominalArgs, class... ActualArgs>
    inline typename detail::CallEnabler<NominalArgs...>::Void::Type
	call(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
        simpleCall(call, rpc::forward<ActualArgs>(args)...);
    }

    /**
     * Helper that forwards results obtained via callback into std::future.
     */
    template<class...> class ResultRetriever {};

    template<class V> class ResultRetriever<V>
    {
        friend StlFacade;
        using Future = std::future<V>;
        mutable std::promise<V> promise;

        inline auto getFuture() {
            return promise.get_future();
        }

    public:
        inline auto operator()(StlFacade::Endpoint& ep, const MethodHandle &h, V v) const
        {
            promise.set_value(std::move(v));
            return ep.uninstall(h);
        }
    };

    /**
     * Wrapper for Endpoint::call, value returned via callback case.
     * 
     * Can only be used when the last argument is a callback handle.
     * Expects to called with the arguments of thy signature except the last one.
     * 
     * Returns an std::future object that gets set to the returned value when
     * the callback is invoked.
     * 
     * Throws instead of returning error.
     */
    template<class... NominalArgs, class... ActualArgs>
	inline typename detail::CallEnabler<NominalArgs...>::template Retriever<ResultRetriever>::Future 
    call(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
        typename detail::CallEnabler<NominalArgs...>::template Retriever<ResultRetriever> retriever;       

        auto ret = retriever.getFuture();
        auto id = this->StlFacade::Endpoint::install(std::move(retriever));

        if(auto err = this->StlFacade::Endpoint::call(call, std::forward<ActualArgs>(args)..., id))
        {
            assert(this->StlFacade::Endpoint::uninstall(id));
            throw RpcException(err);
        }

        return ret;
    }
};

}

#endif /* _RPCSTLFACADE_H_ */

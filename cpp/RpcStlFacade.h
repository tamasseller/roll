#ifndef _RPCSTLFACADE_H_
#define _RPCSTLFACADE_H_

#include "RpcEndpoint.h"

#include <memory>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <future>

#include <assert.h>
#include <string.h>

namespace rpc {

namespace detail
{   
    template<class K, class V>
    class HashMapRegistry
    {
        std::unordered_map<K, V> lookupTable;
        std::mutex mut;

    public:
        inline bool remove(const K& k) 
        {
            std::lock_guard _(mut);

            auto it = lookupTable.find(k);

            if(it == lookupTable.end())
                return false;

            lookupTable.erase(it);
            return true;
        }

        inline bool add(const K& k, V&& v) 
        {
            std::lock_guard _(mut);

            auto it = lookupTable.find(k);

            if(it != lookupTable.end())
                return false;

            return lookupTable.emplace(k, std::move(v)).second;
        }

        inline V* find(const K& k, bool &ok) 
        {
            std::lock_guard _(mut);

            auto it = lookupTable.find(k);

            if(it == lookupTable.end())
            {
                ok = false;
                return nullptr;
            }
            
            ok = true;
            return &it->second;
        }
    };

    template<class T>
    struct StlAutoPointer: std::unique_ptr<T> 
    {
        StlAutoPointer(std::unique_ptr<T> &&v): std::unique_ptr<T>(std::move(v)) {}

        template<class U, class... Args>
        static inline StlAutoPointer make(Args&&... args) {
            return StlAutoPointer(std::unique_ptr<T>(new U(std::forward<Args>(args)...)));
        }
    };

    struct HashMapBasedNameDictionary
    {
        std::unordered_map<std::string, uint32_t> fwd;
        std::unordered_map<uint32_t, std::string> bwd;
        std::mutex mut;

        class Query: std::stringstream
        {
            friend HashMapBasedNameDictionary;

        public:
            auto &operator<<(char c) {
                return *((std::stringstream*)this) << c;
            }

            bool run(HashMapBasedNameDictionary& self, uint32_t &result) 
            {
                std::lock_guard _(self.mut);

                auto it = self.fwd.find(this->str());

                if(it == self.fwd.end())
                    return false;

                result = it->second;
                return true;
            }
        };

        Query beginQuery() { return {}; }

        bool addMapping(const char* name, uint32_t value) 
        {
            std::lock_guard _(mut);

            auto [it, done] = fwd.insert({name, value});

            if(!done)
                return false;

            if(!bwd.insert({value, name}).second)
            {
                fwd.erase(it);
                return false;
            }

            return true;
        }

        bool removeMapping(const char* name, uint32_t &value) 
        {
            std::lock_guard _(mut);

            auto it = fwd.find(name);

            if(it != fwd.end())
            {
                value = it->second;
                fwd.erase(it);

                auto it2 = bwd.find(value);
                assert(it2 != bwd.end());
                bwd.erase(it2);
                return true;
            }

            return false;
        }

        bool removeMapping(uint32_t value) 
        {
            std::lock_guard _(mut);

            auto it = bwd.find(value);

            if(it != bwd.end())
            {
                auto name = it->second;
                bwd.erase(it);

                auto it2 = fwd.find(name);
                assert(it2 != fwd.end());
                fwd.erase(it2);
                return true;
            }

            return false;
        }
    };

    template<class...> struct CallEnabler;

    template<class... CbArgs>
    struct CallEnabler<Call<CbArgs...>>
    {
        template<template<class...> class R> using Retriever = R<CbArgs...>;
        struct Void {};
    };

    template<>
    struct CallEnabler<>
    {
        template<template <class...> class> class Retriever {};
        struct Void { using Type = void; };
    };

    template<class C>
    struct CallEnabler<C>
    {
        template<template <class...> class> class Retriever {};
        struct Void { using Type = void; };
    };

    template<class First, class... Rest>
    struct CallEnabler<First, Rest...>
    {
        template<template<class...> class R> using Retriever = typename CallEnabler<Rest...>::template Retriever<R>;
        using Void = typename CallEnabler<Rest...>::Void;
    };
}

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
public:
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
    inline typename detail::CallEnabler<NominalArgs...>::Void::Type 
	call(const Call<NominalArgs...> &call, ActualArgs&&... args)
	{
        if(auto err = this->StlFacade::Endpoint::call(call, std::forward<ActualArgs>(args)...))
            throw RpcException(err);
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

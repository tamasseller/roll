#ifndef RPC_CPP_RPCSTLADAPTERS_H_
#define RPC_CPP_RPCSTLADAPTERS_H_

#include "RpcFail.h"
#include "RpcEndpoint.h"

#include <memory>
#include <sstream>
#include <unordered_map>
#include <mutex>

#include <cassert>

namespace rpc {

template<class... CbArgs> class Call;

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
}

/**
 * Facade for the generic RPC endpoint using STL based implementation of auxiliary operations.
 *
 * This is supposed to be the regular front-end class for PC-like environments - i.e. wherever
 * using the STL classes is advisable. This also means that dynamic memory usage is managed by
 * the STL implementation. When tighter control over heap usage is a requirement alternate
 * implementations for the dependencies can be used.
 */
template<class Io>
class StlEndpoint:
	public Io,
	public Endpoint<
		detail::StlAutoPointer,
		detail::HashMapRegistry,
		typename Io::InputAccessor,
		StlEndpoint<Io>
	>
{
public:
    /**
     * Wrapper around Endpoint::init.
     */
    template<class... Args>
    StlEndpoint(Args&&... args): Io(std::forward<Args>(args)...)
	{
    	if(!this->StlEndpoint::Endpoint::init())
    	{
    		fail("could not initialize RPC endpoint object");
    	}
    }
};

}

#endif /* RPC_CPP_RPCSTLADAPTERS_H_ */

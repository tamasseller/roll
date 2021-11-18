#ifndef RPC_CPP_RPCSTLADAPTERS_H_
#define RPC_CPP_RPCSTLADAPTERS_H_

#include "common/Fail.h"

#include "base/RpcEndpoint.h"

#include "types/Collection.h"

#include <memory>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <vector>

#include <cassert>

namespace rpc {

template<class... CbArgs> class Call;

template<> struct CollectionSelector<int8_t, void> {
	using Type = std::string;
};

template<class C> struct CollectionSelector<C, void> {
	using Type = std::vector<C>;
};

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
    explicit StlEndpoint(Args&&... args): Io(std::forward<Args>(args)...)
	{
    	if(!this->StlEndpoint::Endpoint::init())
    	{
    		fail("could not initialize RPC endpoint object");
    	}
    }
};

}

#endif /* RPC_CPP_RPCSTLADAPTERS_H_ */

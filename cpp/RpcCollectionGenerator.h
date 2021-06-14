#ifndef RPC_CPP_RPCCOLLECTIONGENERATOR_H_
#define RPC_CPP_RPCCOLLECTIONGENERATOR_H_

#include "RpcTypeInfo.h"

namespace rpc {

/**
 * Utility for dynamic generation of a collection via a functor (e.g.
 * a lambda object).
 *
 * Specifying it as an argument to a remote method invocation enables
 * the serialization of homogeneous synthetic data as a collection.
 */
template<class T, class C>
class CollectionGenerator
{
    class Terminate{};

    class Generator
    {
        friend CollectionGenerator;

        uint32_t remaining;
    	C generator;

        /**
         * Constructor used internally by the begin iterator getter.
         */
        inline Generator(uint32_t remaining, C&& c): remaining(remaining), generator(rpc::move(c)) {}

    public:
        /**
         * STL (and range based for expression) compatible value access operator.
         *
         * Uses the read method, discards the error if there is any. On error returns
         * the default constructed value of the element type.
         */
        inline T operator*()
        {
			if(remaining)
			{
				--remaining;
				return generator();
			}

			return {};
        }

        /**
         * STL (and range based for expression) iterator step operator.
         *
         * Does nothing because the value access already moves the forward.
         */
        inline auto& operator++() { return *this; }

        /**
         * STL (and range based for expression) inequality against dummy end marker operator.
         *
         * Returns true of there are elments still to be read.
         */
        inline bool operator!=(const Terminate&) const { return remaining > 0; }
    };

    const Generator generator;

public:
    /**
     * Construct from a block of memory (for example an array).
     *
     * A pointer to the first element and the number of elements are needed to be specified.
     */
    inline CollectionGenerator(uint32_t length, C&& c): generator(length, rpc::move(c)) {}

    /**
     * STL container like size getter.
     *
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase.
     */
    size_t size() const { return generator.remaining; }

    /**
     * STL container like begin iterator getter.
     *
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase.
     */
    inline auto begin() const {
    	return generator;
    }

    /**
     * STL container like end iterator getter.
     *
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase.
     */
    Terminate end() const { return {}; }
};

/**
 * Helper method for creating a collection generator.
 */
template<class C>
static auto generateCollection(uint32_t length, C &&c)
{
	using T = decltype(c());
	return CollectionGenerator<T, C>(length, rpc::move(c));
}

/**
 * Serialization rules for CollectionGenerator.
 */
template<class T, class C> struct TypeInfo<CollectionGenerator<T, C>>: StlCompatibleCollectionTypeBase<CollectionGenerator<T, C>, T>
{
    template<class A> static inline bool write(A& a, const CollectionGenerator<T, C> &v)
    {
    	auto length = v.size();

        if(!VarUint4::write(a, length))
            return false;

        for(T d: v)
            if(!TypeInfo<T>::write(a, rpc::move(d)))
                return false;

        return true;
    }
};

}

#endif /* RPC_CPP_RPCCOLLECTIONGENERATOR_H_ */

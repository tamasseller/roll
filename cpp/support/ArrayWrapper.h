#ifndef ROLL_CPP_SUPPORT_ARRAYWRAPPER_H_
#define ROLL_CPP_SUPPORT_ARRAYWRAPPER_H_

#include "types/CollectionTypeInfoCommon.h"

#include "base/VarInt.h"

namespace rpc {

/**
 * Direct (by-value) wrapper for a collection value - with a predefined maximal number of inline elements.
 *
 * Specifying it as an argument to a remote method invocation enables the serialization of (small)
 * calculated arrays of elements without dynamic memory management.
 */
template<class T, size_t max>
struct ArrayWrapper
{
    const T data[max];
    const size_t length;

    template<size_t... i>
    inline constexpr ArrayWrapper(const sequence<i...>&, const T (&input)[sizeof...(i)]):
		data{input[i]..., }, length(sizeof...(i)) {}

public:
    /**
     * Construct from element values.
     */
    template<size_t n>
    inline constexpr ArrayWrapper(const T (&input)[n]): ArrayWrapper(indices<n>{}, input) {
    	static_assert(n <= max);
    }

    /**
     * Construct an array of size 1 from single element value.
     */
	inline constexpr ArrayWrapper(const T &input): data{input,}, length(1) {}

    /**
     * Copy an array of a size not greater than the maximal size.
     */
	template<size_t n>
	inline constexpr ArrayWrapper(const ArrayWrapper<T, n> &o): ArrayWrapper(indices<n>{}, o.data) {
    	static_assert(n <= max);
    }

    /**
     * STL container like size getter.
     *
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase.
     */
    size_t size() const { return length; }

    /**
     * STL container like begin iterator getter.
     *
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase.
     */
    const T* begin() const { return data; }

    /**
     * STL container like end iterator getter.
     *
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase.
     */
    const T* end() const { return data + length; }
};

template<class T> ArrayWrapper(const T&) -> ArrayWrapper<T, 1>;
template<class T, size_t n> ArrayWrapper(const T (&v)[n]) -> ArrayWrapper<T, n>;

/**
 * Serialization rules for ArrayWrapper.
 */
template<class T, size_t n> struct TypeInfo<ArrayWrapper<T, n>>: StlCompatibleCollectionTypeBase<ArrayWrapper<T, n>, T>
{
    template<class A> static inline bool write(A& a, const ArrayWrapper<T, n> &v)
    {
        if(!VarUint4::write(a, v.length))
            return false;

        for(auto i = 0u; i < v.length; i++)
            if(!TypeInfo<T>::write(a, v.data[i]))
                return false;

        return true;
    }
};

}

#endif /* ROLL_CPP_SUPPORT_ARRAYWRAPPER_H_ */

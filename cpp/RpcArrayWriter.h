#ifndef _RPCARRAYWRITER_H_
#define _RPCARRAYWRITER_H_

#include "RpcTypeInfo.h"

namespace rpc {

/**
 * Indirect writer for array backed collection.
 * 
 * Specifying it as an argument to a remote method invocation enables the serialization
 * of a plain block of (probably constant or heap) memory as a collection.
 */
template<class T>
class ArrayWriter
{
    friend TypeInfo<ArrayWriter<T>>;
    const T* data;
    uint32_t length;

public:
    inline ArrayWriter() = default;

    /**
     * Construct from a block of memory (for example an array).
     * 
     * A pointer to the first element and the number of elements are needed to be specified.
     */
    inline ArrayWriter(const T* data, uint32_t length): data(data), length(length) {}

    /**
     * Construct from a reference to an actual array.
     */
    template<class U, size_t n>
    inline ArrayWriter(const U (&data)[n]): data((const T*)data), length(n) {
    	static_assert(sizeof(U) == sizeof(T));
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
    auto begin() const { return data; }

    /**
     * STL container like end iterator getter.
     * 
     * Needed for STL compatibility, which allows for reuse of StlCompatibleCollectionTypeBase. 
     */
    auto end() const { return data + length; }
};

/**
 * Serialization rules for ArrayWriter.
 */
template<class T> struct TypeInfo<ArrayWriter<T>>: StlCompatibleCollectionTypeBase<ArrayWriter<T>, T> 
{
    template<class A> static inline bool write(A& a, const ArrayWriter<T> &v) 
    { 
        if(!VarUint4::write(a, v.length))
            return false;

        for(auto i = 0u; i < v.length; i++)
            if(!TypeInfo<T>::write(a, v.data[i]))
                return false;

        return true;    
    }
};

/**
 * Direct (by-value) wrapper for a collection value - with a predefined maximal number of inline elements.
 *
 * Specifying it as an argument to a remote method invocation enables the serialization of (small)
 * calculated arrays of elements without dynamic memory management.
 */
template<class T, size_t max = 1>
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

template<class T> ArrayWrapper(const T&) -> ArrayWrapper<T>;

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

#endif /* _RPCARRAYWRITER_H_ */

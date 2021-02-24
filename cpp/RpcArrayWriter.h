#ifndef _RPCARRAYWRITER_H_
#define _RPCARRAYWRITER_H_

#include "RpcTypeInfo.h"

namespace rpc {

/**
 * Writer for array backed collection.
 * 
 * Specifying it as an argument to a remote method invocation enables the 
 * serialization of a plain block of memory as a colletion.
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
    template<size_t n>
    inline ArrayWriter(const T (&data)[n]): data(data), length(n) {}

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

}
#endif /* _RPCARRAYWRITER_H_ */

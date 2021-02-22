#ifndef _RPCARRAYWRITER_H_
#define _RPCARRAYWRITER_H_

#include "RpcTypeInfo.h"

namespace rpc {

template<class T>
class ArrayWriter
{
    friend TypeInfo<ArrayWriter<T>>;
    const T* data;
    uint32_t length;

public:
    inline ArrayWriter() = default;
    inline ArrayWriter(const T* data, uint32_t length): data(data), length(length) {}

    template<size_t n>
    inline ArrayWriter(const T (&data)[n]): data(data), length(n) {}

    size_t size() const { return length; }
    auto begin() const { return data; }
    auto end() const { return data + length; }
};

template<class T> struct TypeInfo<ArrayWriter<T>>: StlCompatibleCollectionTypeBase<ArrayWriter<T>, T> 
{
    template<class A> static inline bool write(A& a, const ArrayWriter<T> &v) 
    { 
        if(!VarUint4::write(a, v.length))
            return false;

        for(int i = 0; i < v.length; i++)
            if(!TypeInfo<T>::write(a, v.data[i]))
                return false;

        return true;    
    }
};

}
#endif /* _RPCARRAYWRITER_H_ */

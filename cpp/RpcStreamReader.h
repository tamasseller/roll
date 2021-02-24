#ifndef _RPCSTLSTREAMREADER_H_
#define _RPCSTLSTREAMREADER_H_

#include "RpcTypeInfo.h"

namespace rpc {

template<class T, class A>
class StreamReader
{
    A accessor;
    uint32_t length = 0;

public:
    inline StreamReader() = default;
    inline StreamReader(A accessor, uint32_t length): accessor(accessor), length(length) {};

    struct StreamEnd{};
    inline auto end() { return StreamEnd{}; }

    inline auto size() const {
        return length;
    }

    class Cursor
    {
        A accessor;
        uint32_t remaining;

        friend StreamReader;
        inline Cursor(const A &accessor, uint32_t remaining): accessor(accessor), remaining(remaining) {}

    public:
        inline bool read(T &v)
        {
            if(remaining)
            {
                --remaining;
                return TypeInfo<T>::read(accessor, v);
            }

            return false;
        }

        inline T operator*() 
        {
            T v;
            this->read(v);
            return v;
        };

        inline auto& operator++() { return *this; }
        inline bool operator!=(const StreamEnd&) const { return remaining > 0; }
    };
    
    auto begin() { return Cursor(accessor, length); }
};

template<class T, class A> struct TypeInfo<StreamReader<T, A>>: CollectionTypeBase<T> 
{
    static inline bool read(A& a, StreamReader<T, A> &v) 
    { 
        uint32_t count;

        if(!VarUint4::read(a, count))
            return false;

        v = StreamReader<T, A>(a, count);

        for(auto i = 0u; i < count; i++)
            if(!TypeInfo<T>::skip(a))
                return false;

        return true;    
    }
};

}

#endif /* _RPCSTLSTREAMREADER_H_ */

#ifndef _RPCSTLSTREAMREADER_H_
#define _RPCSTLSTREAMREADER_H_

#include "types/TypeInfo.h"

namespace rpc {

/**
 * Streaming collection reader, used for zero-copy deserialization.
 * 
 * Specifying it as an argument of a remotely callable method allows it to 
 * read a collection lazily - i.e. without needing its members to be parsed
 * during deserialization. It allows the method to parse the elements of the 
 * collection while iterating through it.
 */
template<class T, class A>
class StreamReader
{
    A accessor;
    uint32_t length = 0;

public:
    inline StreamReader() = default;

    /**
     * Constructor used internally during deserialization.
     * 
     * Stores a stream accessor to the beginning of the first element of 
     * the collection and the number of elements.
     */
    inline StreamReader(A accessor, uint32_t length): accessor(accessor), length(length) {};

    /**
     * Dummy end iterator type.
     * 
     * Used as a return type for end iterator getter.
     */
    struct StreamEnd{};

    /**
     * End iterator getter.
     * 
     * Needed for compatibility with range based for loop expressions.
     */
    static inline auto end() { return StreamEnd{}; }

    /**
     * STL-like size getter.
     * 
     * Convenience method.
     */
    inline auto size() const {
        return length;
    }

    /**
     * Actual iterator type.
     * 
     * Returned by begin iterator getter.
     */
    class Cursor
    {
        /**
         * Accessor to read from the input stream.
         */
        A accessor;

        /**
         * Number of remaining elements. 
         */
        uint32_t remaining;

        friend StreamReader;

        /**
         * Constructor used internally by the begin iterator getter.
         * 
         * Copies the contents of the StreamReader: accessor to the first
         * element and the number of elements.
         */
        inline Cursor(const A &accessor, uint32_t remaining): accessor(accessor), remaining(remaining) {}

    public:
        /**
         * Try to read an element.
         * 
         * Stores the result via the reference passed to it as argument.
         * 
         * Returns true on success, false if there was no element to 
         * read or if the element could not have been parsed.
         */
        inline bool read(T &v)
        {
            if(remaining)
            {
                --remaining;
                return TypeInfo<T>::read(accessor, v);
            }

            return false;
        }

        /**
         * STL (and range based for expression) compatible value access operator.
         * 
         * Uses the read method, discards the error if there is any. On error returns
         * the default constructed value of the element type.
         */
        inline T operator*() 
        {
            T v;
            this->read(v);
            return v;
        };

        /**
         * STL (and range based for expression) iterator step operator.
         * 
         * Does nothing because the value access already moves the forward.
         */
        inline auto& operator++() { return *this; }

        /**
         * STL (and range based for expression) iterator step operator.
         *
         * Does nothing because the value access already moves the forward.
         */
        inline auto& operator++(int) { return *this; }

        /**
         * STL (and range based for expression) inequality against dummy end marker operator.
         * 
         * Returns true of there are elments still to be read.
         */
        inline bool operator!=(const StreamEnd&) const { return remaining > 0; }
    };
    
    /**
     * Begin iterator getter.
     * 
     * Needed for compatibility with range based for loop expressions.
     */
    auto begin() const { return Cursor(accessor, length); }

    /**
     * Helper that copies at most _n_ of the initial elements from the collection
     *
     * Returns the number of elements copied.
     */
    inline size_t copy(T* out, size_t n) const
    {
    	size_t idx = 0;

    	for(auto it = begin(); (idx < n) && it != end(); idx++, it++)
    	{
    		*out++ = *it;
    	}

    	return idx;
    }
};

/**
 * Serialization rules for StreamReader.
 */
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

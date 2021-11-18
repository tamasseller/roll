#ifndef _RPCFDSTREAMADAPTER_H_
#define _RPCFDSTREAMADAPTER_H_

#include "StlAdapters.h"

#include <memory>
#include <list>

#include <cassert>
#include <cstring>

#include <unistd.h>

namespace rpc {

class FdStreamAdapter;
class PreallocatedMemoryBufferStreamWriterFactory;

class PreallocatedMemoryBufferStream
{
    std::unique_ptr<char[]> buffer;
    char *start, *end;

    friend FdStreamAdapter;
    friend PreallocatedMemoryBufferStreamWriterFactory;

    inline PreallocatedMemoryBufferStream(std::unique_ptr<char[]> &&buffer, size_t size): 
        buffer(std::move(buffer)), start(this->buffer.get()), end(this->buffer.get() + size) {}
    
public:
    struct Accessor
    {
        friend PreallocatedMemoryBufferStream;
        char *ptr = nullptr, *end = nullptr;

        friend class PreallocatedMemoryBufferStreamWriter;

    public:
        inline Accessor(char* ptr, char* end): ptr(ptr), end(end) {}
        inline Accessor() = default;
        
        template<class T>
        bool write(const T& v)
        {
            constexpr auto size = sizeof(T);
            assert(size <= size_t(end - ptr));
            memcpy(ptr, &v, size);
            ptr += size;
            return true;
        }

        template<class T>
        bool read(T& v)
        {
            constexpr auto size = sizeof(T);
            assert(size <= size_t(end - ptr));
            memcpy(&v, ptr, size);
            ptr += size;
            return true;
        }

        bool skip(size_t size)
        {
            assert(size <= size_t(end - ptr));
            ptr += size;
            return true;
        }
    };

    inline auto access() {
        return Accessor(start, end);
    }

    inline PreallocatedMemoryBufferStream(PreallocatedMemoryBufferStream&&) = default;
    inline PreallocatedMemoryBufferStream& operator =(PreallocatedMemoryBufferStream&&) = default;
    inline PreallocatedMemoryBufferStream(size_t size):
        buffer(new char[size]),
        start(buffer.get()), end(buffer.get() + size)
    {
        auto a = access();
        assert(size == (size_t)((uint32_t)size));
        auto lengthWriteOk = rpc::VarUint4::write(a, size);
        assert(lengthWriteOk);
        start = a.ptr;
    };
};

struct PreallocatedMemoryBufferStreamWriter: PreallocatedMemoryBufferStream, PreallocatedMemoryBufferStream::Accessor {
    inline PreallocatedMemoryBufferStreamWriter(size_t s): 
        PreallocatedMemoryBufferStream(s + VarUint4::size((uint32_t)s)),
        PreallocatedMemoryBufferStream::Accessor(this->access()) {}
};

struct PreallocatedMemoryBufferStreamWriterFactory
{
    using Accessor = PreallocatedMemoryBufferStream::Accessor;

    static inline auto build(size_t s) {
        return PreallocatedMemoryBufferStreamWriter(s); 
    }

    static inline auto done(PreallocatedMemoryBufferStreamWriter &&w) 
    {
        auto stream = static_cast<PreallocatedMemoryBufferStream&&>(w);
        auto accessor = static_cast<PreallocatedMemoryBufferStream::Accessor&&>(w);
        stream.end = accessor.end;
        return stream; 
    }
};

class FdStreamAdapter
{
    int wfd = -1, rfd = -1;

public:
    using InputAccessor = PreallocatedMemoryBufferStream::Accessor;

    FdStreamAdapter(const FdStreamAdapter&) = delete;
    inline FdStreamAdapter(int wfd, int rfd): wfd(wfd), rfd(rfd) {}

    inline auto messageFactory() {
    	return PreallocatedMemoryBufferStreamWriterFactory{};
    }

    bool send(PreallocatedMemoryBufferStream&& data)
    {
        auto ptr = data.buffer.get();
        auto len = data.end - ptr;
        return write(wfd, ptr, len) == len;
    }

    template<class C>
    bool receive(C&& cb)
    {
        uint32_t messageLength;
        VarUint4::Reader r;

        while(true)
        {
            char c;
            if(read(rfd, &c, 1) != 1)
                return false;

            if(r.process(c))
            {
                auto result = r.getResult();
                messageLength = result - VarUint4::size((uint32_t)result);
                break;
            }
        }

        std::unique_ptr<char[]> buffer(new char[messageLength]);

        if(read(rfd, buffer.get(), messageLength) != messageLength)
            return false;

        return cb(PreallocatedMemoryBufferStream(std::move(buffer), messageLength));
    }
};

}

#endif /* _RPCFDSTREAMADAPTER_H_ */

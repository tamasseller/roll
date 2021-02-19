#ifndef _MOCKSTREAM_H_
#define _MOCKSTREAM_H_

#include <memory>
#include <string.h>

class MockStream
{
    std::unique_ptr<char[]> buffer;
    char *end;

    class Accessor
    {
        friend MockStream;
        char *ptr, *const end;
        inline Accessor(char* ptr, char* end): ptr(ptr), end(end) {}

    public:
        template<class T>
        bool write(const T& v)
        {
            static constexpr auto size = sizeof(T);
            if(size <= end - ptr)
            {
                memcpy(ptr, &v, size);
                ptr += size;
                return true;
            }

            return false;
        }

        template<class T>
        bool read(T& v)
        {
            static constexpr auto size = sizeof(T);
            if(size <= end - ptr)
            {
                memcpy(&v, ptr, size);
                ptr += size;
                return true;
            }

            return false;
        }

        template<class T>
        bool skip(size_t size)
        {
            if(size <= end - ptr)
            {
                ptr += size;
                return true;
            }

            return false;
        }
    };

public:
    inline auto access() {
        return Accessor(buffer.get(), end);
    }

    inline bool truncateAt(size_t offset)
    {
        auto start = buffer.get();

        if(offset < end - start)
        {
            end = start + offset;
            return true;
        }

        return false;
    }

    inline MockStream(size_t size): buffer(new char[size]), end(buffer.get() + size) {};
};

#endif /* _MOCKSTREAM_H_ */

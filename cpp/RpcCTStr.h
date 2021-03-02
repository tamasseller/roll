#ifndef _RPCCTSTR_H_
#define _RPCCTSTR_H_

#include <stddef.h>

namespace rpc {

/**
 * Compile Time String.
 * 
 * Helper class used for generation of method signatures.
 */
template<size_t length>
class CTStr
{
    char data[length + 1];
    template<size_t n> friend class CTStr;
    
public:
    static constexpr size_t strLength = length;

    template<size_t n1, size_t n2>
    constexpr CTStr(const CTStr<n1>& s1, const char(&arr)[n2]): data{0}
    {
		static_assert(n1 + n2 - 1 == length);
		
		for(auto i = 0u; i < n1; i++)
			data[i] = s1.data[i];

		for(auto i = 0u; i < n2; i++)
			data[n1 + i] = arr[i];
	}

    constexpr CTStr(const char(&arr)[length + 1]): data{0}
    {
        for(auto i = 0u; i < length + 1; i++)
			data[i] = arr[i];
    }

    constexpr CTStr(const CTStr&) = default;

    constexpr operator const char *() const { return data; }
    constexpr size_t size() const { return size; }

    template<size_t oLen>
    constexpr bool operator ==(const CTStr<oLen>&o) const 
    {
        if(oLen != length)
            return false;

        for(auto i = 0u; i < oLen; i++)
            if(o.data[i] != data[i])
                return false;
        
        return true;
    }
};

template <size_t n1, size_t n2>
static inline constexpr CTStr<n1 + n2 - 1> operator<<(const CTStr<n1>& s1, const char(&a2)[n2]) {
	return CTStr<n1 + n2 - 1>(s1, a2);
}

template<size_t np1> CTStr(const char (&)[np1]) -> CTStr<np1 - 1>;

}

#endif /* _RPCCTSTR_H_ */

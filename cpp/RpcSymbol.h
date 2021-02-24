#ifndef _RPCSYMBOL_H_
#define _RPCSYMBOL_H_

#include "RpcSignatureGenerator.h"
#include "RpcCTStr.h"
#include <stddef.h>

namespace rpc {
    
template<size_t n, class... Args>
struct Symbol: CTStr<n>
{
    using CallType = Call<Args...>;
    using Symbol::CTStr::operator const char*;
    inline constexpr Symbol(const CTStr<n> &name): CTStr<n>(name) {}
};

template<class... Args, size_t n>
inline constexpr auto symbol(const CTStr<n> &name)
{
    auto sgn = writeSignature<Args...>(name);
    constexpr auto length = decltype(sgn)::strLength;
    return Symbol<length, Args...>(sgn);
}

}
#endif /* _RPCSYMBOL_H_ */

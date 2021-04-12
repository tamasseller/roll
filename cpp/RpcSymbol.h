#ifndef _RPCSYMBOL_H_
#define _RPCSYMBOL_H_

#include "RpcSignatureGenerator.h"
#include "RpcCTStr.h"

#include <stddef.h>

namespace rpc {

/**
 * Public identifier method identifer for remote lookup.
 * 
 * The Symbol represents a named method and its argument types.
 * It can be used for registration and lookup of public methods.
 */
template<size_t n, class... Args>
struct Symbol: CTStr<n>
{
    using CallType = Call<Args...>;
    using Symbol::CTStr::operator const char*;
    inline constexpr Symbol(const CTStr<n> &name): CTStr<n>(name) {}
};

/**
 * Helper function to build Symbol objects.
 * 
 * It allows compile time construction of the method signature.
 */
template<class... Args, size_t n>
inline constexpr auto symbol(const CTStr<n> &name)
{
    auto sgn = writeSignature<Args...>(name);
    constexpr auto length = decltype(sgn)::strLength;
    return Symbol<length, Args...>(sgn);
}

template<class... Args, size_t n>
inline constexpr auto symbol(const Call<Args...>&, const CTStr<n> &name)
{
	return symbol<Args...>(name);
}

}
#endif /* _RPCSYMBOL_H_ */

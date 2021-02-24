#ifndef _RPCRPCSIGNATUREGENERATOR_H_
#define _RPCRPCSIGNATUREGENERATOR_H_

#include "RpcUtility.h"

namespace rpc {
	
template<class T> struct TypeInfo;

template<class...> struct SignatureGenerator;
template<> struct SignatureGenerator<>
{
	template<class S> static inline constexpr decltype(auto) writeTypes(S&& s) { return rpc::forward<S>(s); }
	template<class S> static inline constexpr decltype(auto) writeNextType(S&& s) { return rpc::forward<S>(s); }
};

template<class First, class... Rest>
struct SignatureGenerator<First, Rest...>
{
	template<class S>
	static inline constexpr decltype(auto) writeNextType(S&& s) {
		return SignatureGenerator<Rest...>::writeNextType(TypeInfo<First>::writeName(s << ","));
	}

	template<class S>
	static inline constexpr decltype(auto) writeTypes(S&& s) { 
		return SignatureGenerator<Rest...>::writeNextType(TypeInfo<First>::writeName(s));
	}
};

template<class... Types, class S>
static inline constexpr decltype(auto) writeSignature(S&& s) {
	return SignatureGenerator<Types...>::writeTypes(s << "(") << ")";
}

}

#endif

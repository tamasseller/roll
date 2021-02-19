#ifndef COMMON_RPCRPCSIGNATUREGENERATOR_H_
#define COMMON_RPCRPCSIGNATUREGENERATOR_H_

#include <utility>

namespace rpc {
	
template<class T> struct TypeInfo;

template<class...> struct SignatureGenerator;
template<> struct SignatureGenerator<>
{
	template<class S> static inline constexpr decltype(auto) writeTypes(S&& s) { return s; }
	template<class S> static inline constexpr decltype(auto) writeNextType(S&& s) { return s; }
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
};

}

#endif

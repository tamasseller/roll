#ifndef _RPCRPCSIGNATUREGENERATOR_H_
#define _RPCRPCSIGNATUREGENERATOR_H_

#include "common/Utility.h"

#include "Call.h"

namespace rpc {

/**
 * Serialization rules.
 * 
 * A specializations of this template class corresponds to all serializable types.
 */	
template<class T> struct TypeInfo;

/**
 * Iteration helper that uses the TypeInfo template class to create a part of a method signature.
 */
template<class...> struct SignatureGenerator;

/**
 * Iteration stopper.
 */
template<> struct SignatureGenerator<>
{
	template<class S> static inline constexpr decltype(auto) writeTypes(S&& s) { return s; }
	template<class S> static inline constexpr decltype(auto) writeNextType(S&& s) { return s; }
};

/**
 * Variadic argument peeler.
 */
template<class First, class... Rest>
struct SignatureGenerator<First, Rest...>
{
	template<class S>
	static inline constexpr decltype(auto) writeNextType(S&& s) {
		auto &&x = TypeInfo<remove_cref_t<First>>::writeName(s << ",");
		return SignatureGenerator<Rest...>::writeNextType(rpc::move(x));
	}

	template<class S>
	static inline constexpr decltype(auto) writeTypes(S&& s) {
		auto &&x = TypeInfo<remove_cref_t<First>>::writeName(s);
		return SignatureGenerator<Rest...>::writeNextType(rpc::move(x));
	}
};

/**
 * Helper to create signature for a method.
 */
template<class... Types, class S>
static inline constexpr decltype(auto) writeSignature(S&& s) {
	return SignatureGenerator<Types...>::writeTypes(s << "(") << ")";
}

}

#endif

#ifndef COMMON_RPCRPCSIGNATUREGENERATOR_H_
#define COMMON_RPCRPCSIGNATUREGENERATOR_H_

namespace rpc {
	
template<class T> struct TypeInfo;

template<class S, class...> struct SignatureGenerator;

template<class S> struct SignatureGenerator<S>
{
	static inline S& writeTypes(S& s) { return s;}
	static inline S& writeNextType(S& s) { return s;}
};

template<class S, class First, class... Rest>
struct SignatureGenerator<S, First, Rest...>
{
	static inline S& writeNextType(S& s) {
		return SignatureGenerator<S, Rest...>::writeNextType(TypeInfo<First>::writeName(s << ","));
	}

	static inline S& writeTypes(S& s) { 
		return SignatureGenerator<S, Rest...>::writeNextType(TypeInfo<First>::writeName(s));
	}
};

template<class... Types, class S>
static inline auto &writeSignature(S& s) {
	return SignatureGenerator<decltype(s << "("), Types...>::writeTypes(s << "(") << ")";
};

}

#endif

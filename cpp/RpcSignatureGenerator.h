#ifndef COMMON_RPCRPCSIGNATUREGENERATOR_H_
#define COMMON_RPCRPCSIGNATUREGENERATOR_H_

template<class T> struct RpcTypeInfo;

template<class S, class...> struct RpcSignatureGenerator;

template<class S> struct RpcSignatureGenerator<S>
{
	static inline S& writeTypes(S& s) { return s;}
	static inline S& writeNextType(S& s) { return s;}
};

template<class S, class First, class... Rest>
struct RpcSignatureGenerator<S, First, Rest...>
{
	static inline S& writeNextType(S& s) {
		return RpcSignatureGenerator<S, Rest...>::writeNextType(RpcTypeInfo<First>::writeName(s << ","));
	}

	static inline S& writeTypes(S& s) { 
		return RpcSignatureGenerator<S, Rest...>::writeNextType(RpcTypeInfo<First>::writeName(s));
	}
};

template<class... Types, class S>
static inline auto &writeSignature(S& s) {
	return RpcSignatureGenerator<decltype(s << "("), Types...>::writeTypes(s << "(") << ")";
};

#endif

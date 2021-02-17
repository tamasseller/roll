#ifndef COMMON_RPCRPCSIGNATUREGENERATOR_H_
#define COMMON_RPCRPCSIGNATUREGENERATOR_H_

#include "RpcTypeInfo.h"
#include "RpcCall.h"

#include <sstream>

template<class...>struct RpcSignatureGenerator;

template<> struct RpcSignatureGenerator<>
{
	static inline void writeNextType(std::ostringstream &ss)
	{
		ss << ")";
	}

	static inline std::string generateSignature(const char* name)
	{
		std::stringstream ss;
		ss << name << "()";
		return ss.str();
	}
};

template<class First, class... Rest>
struct RpcSignatureGenerator<First, Rest...>
{
	static inline void writeNextType(std::ostringstream &ss)
	{
		ss << ", " << RpcTypeInfo<typename std::remove_reference<First>::type>::name;
		RpcSignatureGenerator<Rest...>::writeNextType(ss);
	}

	static inline std::string generateSignature(const char* name)
	{
		std::ostringstream ss;
		ss << name << "(" << RpcTypeInfo<typename std::remove_reference<First>::type>::name;
		RpcSignatureGenerator<Rest...>::writeNextType(ss);
		return ss.str();
	}
};

template<class... Args, class... Rest>
struct RpcSignatureGenerator<RpcCall<Args...>, Rest...>
{
	static inline void writeNextType(std::ostringstream &ss)
	{
		ss << ", " << RpcSignatureGenerator<Args...>::generateSignature("");
		RpcSignatureGenerator<Rest...>::writeNextType(ss);
	}

	static inline std::string generateSignature(const char* name)
	{
		std::ostringstream ss;
		ss << name << "(" << RpcSignatureGenerator<Args...>::generateSignature("");
		RpcSignatureGenerator<Rest...>::writeNextType(ss);
		return ss.str();
	}
};

#endif

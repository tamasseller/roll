#ifndef COMMON_RPCSERDES_H_
#define COMMON_RPCSERDES_H_

#include "RpcCall.h"

#include <assert.h>
#include <type_traits>
#include <utility>

struct RpcSerializer
{
	template<class T>
	inline void serialize(char* &p, char* const end, T&& v)
	{
		auto newp = p + sizeof(T);
		assert(newp <= end);

		*((typename std::remove_reference<T>::type*)p) = v;

		p = newp;
	}

	template<class... Args>
	inline void serialize(char* &p, char* const end, RpcCall<Args...>&& v) {
		serialize(p, end, std::move(v.id));
	}

	void serialize(char* &p, char* const end, const char* &&v);
};

struct RpcDeserializer
{
	template<class T>
	inline void deserialize(char* &p, char* const end, T&& v)
	{
		auto newp = p + sizeof(T);
		assert(newp <= end);

		v = *((typename std::remove_reference<T>::type*)p);

		p = newp;
	}

	template<class... Args>
	inline void deserialize(char* &p, char* const end, RpcCall<Args...>&& v) {
		deserialize(p, end, std::move(v.id));
	}

	void deserialize(char* &p, char* const end, const char* &&v);
};

#endif /* COMMON_RPCSERDES_H_ */

#ifndef _RPCCALL_H_
#define _RPCCALL_H_

#include "common/CTStr.h"

#include "SignatureGenerator.h"

#include <stdint.h>
#include <stddef.h>

namespace rpc {

namespace detail {
	template<class> class CallOperatorSignatureUtility;
}

class MethodHandle;
template<template<class> class, template<class, class> class, class, class, class> class Endpoint;
template<class T> struct TypeInfo;
struct CallIdTestAccessor;

/**
 * Transferable remote procedure handle.
 * 
 * It is used to identify a method for the purpose of remotely initiated
 * invocation. A Call object can be acquired by symbol based lookup or
 * argument passing from the remote end and it can also be created locally
 * by installing a method that can be executed later by the remote end.
 */
template<class... Args> 
class Call
{
	uint32_t id = -1u;

    constexpr inline Call(uint32_t id): id(id) {}

	friend MethodHandle;
	friend CallIdTestAccessor;
	template<class...> friend class Call;
	template<class> friend struct TypeInfo;
	template<template<class> class, template<class, class> class, class, class, class> friend class Endpoint;

public:
    constexpr inline Call() = default;

    template<class... OArgs>
	constexpr inline Call(const Call<OArgs...>& o): id(o.id) {
		static_assert(writeSignature<Args...>(""_ctstr) == writeSignature<OArgs...>(""_ctstr));
	}

	template<class... OArgs>
	constexpr inline Call& operator =(const Call<OArgs...>& o) 
	{
        static_assert(writeSignature<Args...>(""_ctstr) == writeSignature<OArgs...>(""_ctstr));
		id = o.id;
        return *this;
	}

	constexpr inline bool operator==(const Call& other) const {
		return id == other.id;
	}
};

/**
 * Untyped method reference.
 * 
 * To be used only to reference a callback during its own invocation.
 */
class MethodHandle 
{
	uint32_t id;

	template<template<class> class, template<class, class> class, class, class, class> friend class Endpoint;

	template<class... C>
	inline MethodHandle(const Call<C...> &c): id(c.id) {}

public:
	inline MethodHandle(uint32_t id): id(id) {}
};

}

#endif /* _RPCCALL_H_ */

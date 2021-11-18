#ifndef ROLL_CPP_TYPES_CALLTYPEINFO_H_
#define ROLL_CPP_TYPES_CALLTYPEINFO_H_

#include "TypeInfo.h"

#include "base/Call.h"
#include "base/VarInt.h"

namespace rpc {

/**
 * Serialization rules for Call objects.
 *
 * The call object's 32 bit identifier field is encoded using variable length encoding.
 */
template<class... Args> struct TypeInfo<Call<Args...>>
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return writeSignature<Args...>(s); }
	template<class S> static inline bool write(S& s, const Call<Args...> &v) { return ::rpc::VarUint4::write(s, v.id); }
	template<class S> static inline bool read(S& s, Call<Args...> &v) { return ::rpc::VarUint4::read(s, v.id); }
	template<class S> static inline bool skip(S& s) { return ::rpc::VarUint4::skip(s); }
	static constexpr inline size_t size(const Call<Args...> &v) { return ::rpc::VarUint4::size(v.id); }
	static constexpr inline bool isConstSize() { return false; }
};

}

#endif /* ROLL_CPP_TYPES_CALLTYPEINFO_H_ */

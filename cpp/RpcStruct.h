#ifndef RPC_CPP_RPCSTRUCT_H_
#define RPC_CPP_RPCSTRUCT_H_

#include "RpcStlTypes.h"

namespace rpc {

namespace detail {

} // namespace detail

template<auto member> struct StructMember;

template<class Struct, class Type, Type Struct::* mptr> struct StructMember<mptr>
{
	using T = Type;

	static constexpr inline Type& writeAccess(Struct& s) {
		return s.*mptr;
	}

	static constexpr inline const Type& readAccess(const Struct& s) {
		return s.*mptr;
	}
};

/**
 * Parametric serialization rules for structs.
 *
 * NOTE: see AggregateTypeBase for generic rules of aggregate serialization.
 */

template<class Struct, class... Members> struct StructTypeInfo: AggregateTypeBase<typename Members::T...>
{
   	static constexpr inline size_t size(const Struct& v) {
        return (TypeInfo<typename Members::T>::size(Members::readAccess(v)) + ... + 0);
    }

	template<class S> static inline bool write(S& s, const Struct& v) {
        return (TypeInfo<typename Members::T>::write(s, Members::readAccess(v)) && ... && true);
    }

	template<class S> static inline bool read(S& s, Struct& v) {
        return (TypeInfo<typename Members::T>::read(s, Members::writeAccess(v)) && ... && true);
    }
};


}

#endif /* RPC_CPP_RPCSTRUCT_H_ */

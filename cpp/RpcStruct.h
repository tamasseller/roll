#ifndef RPC_CPP_RPCSTRUCT_H_
#define RPC_CPP_RPCSTRUCT_H_

#include "RpcCollection.h"
#include "RpcArrayWriter.h"

namespace rpc {

template<auto member> struct StructMember;

template<class Struct, class Type, Type Struct::* mptr> struct StructMember<mptr>
{
	using T = remove_cref_t<Type>;

	static constexpr inline Type& writeAccess(Struct& s) {
		return s.*mptr;
	}

	static constexpr inline const Type& readAccess(const Struct& s) {
		return s.*mptr;
	}
};

template<auto ptr, auto length> struct DataBlock : decltype(ptr) { };

template<auto ptr, class Struct, class LengthType, LengthType Struct::* len> struct DataBlock<ptr, len>
{
	using ElementType = remove_cref_t<decltype(*(declval<Struct>().*ptr))>;

	using T = rpc::ArrayWriter<ElementType>;

	static constexpr inline auto writeAccess(const Struct& s) {
		return rpc::ArrayWriter<ElementType>(s.*ptr, s.*len);
	}

	static constexpr inline auto readAccess(const Struct& s) {
		return writeAccess(s);
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

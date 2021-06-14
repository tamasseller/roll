#ifndef RPC_CPP_RPCPLAINARRAY_H_
#define RPC_CPP_RPCPLAINARRAY_H_

#include "RpcStlTypes.h"

#include <string>
#include <vector>

namespace rpc {

/**
 * Serialization rules for a simple array.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T, size_t n> struct TypeInfo<T[n]>: CollectionTypeBase<remove_const_t<T>>
{
    static constexpr inline size_t size(const T(&v)[n])
    {
        size_t contentSize = 0;

        if constexpr(TypeInfo<T>::isConstSize())
        {
            contentSize = n * TypeInfo<T>::size(v[0]);
        }
        else
        {
            for(const auto &x: v)
                contentSize += TypeInfo<T>::size(x);
        }

        return contentSize + VarUint4::size(n);
    }

	template<class S> static inline bool write(S& s, const T(&v)[n])
	{
		if(!VarUint4::write(s, n))
			return false;

		for(const auto &x: v)
			if(!TypeInfo<T>::write(s, x))
				return false;

		return true;
	}

	template<class S, class A> static inline bool read(S& s, T(&v)[n], A&& a)
	{
		uint32_t count;
		if(!VarUint4::read(s, count))
			return false;

		if(count != n)
			return false;

		for(auto& x: v)
		{
			if(!TypeInfo<T>::read(s, x))
				return false;
		}

		return true;
	}
};

}

#endif /* RPC_CPP_RPCPLAINARRAY_H_ */

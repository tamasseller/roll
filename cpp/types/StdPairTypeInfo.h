#ifndef ROLL_CPP_TYPES_STDPAIRTYPEINFO_H_
#define ROLL_CPP_TYPES_STDPAIRTYPEINFO_H_

#include "AggregateTypeInfoCommon.h"

#include <utility>

namespace rpc {

/**
 * Serialization rules for std::pair
 *
 * NOTE: see AggregateTypeBase for generic rules of aggregate serialization.
 */
template<class T1, class T2> struct TypeInfo<std::pair<T1, T2>>: AggregateTypeBase<T1, T2>
{
	static constexpr inline size_t size(const std::pair<T1, T2>& v)  {
        return TypeInfo<T1>::size(v.first) + TypeInfo<T2>::size(v.second);
    }

	template<class S> static inline bool write(S& s, const std::pair<T1, T2>& v) {
        return TypeInfo<T1>::write(s, v.first) && TypeInfo<T2>::write(s, v.second);
    }

	template<class S> static inline bool read(S& s, std::pair<T1, T2>& v) {
        return TypeInfo<T1>::read(s, v.first) && TypeInfo<T2>::read(s, v.second);
    }
};

}

#endif /* ROLL_CPP_TYPES_STDPAIRTYPEINFO_H_ */

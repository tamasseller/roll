#ifndef ROLL_CPP_TYPES_AGGREGATETYPEINFOCOMMON_H_
#define ROLL_CPP_TYPES_AGGREGATETYPEINFOCOMMON_H_

#include "TypeInfo.h"

#include "base/SignatureGenerator.h"

namespace rpc {

/**
 * Common serialization rules for struct or tuple like (aggregate) object.
 *
 * An aggregate is encoded simply as its members one after the other.
 */
template<class... Types> struct AggregateTypeBase
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return SignatureGenerator<Types...>::writeTypes(s << "{") << "}";
	}

	template<class S> static inline bool skip(S& s) {
        return (TypeInfo<Types>::skip(s) && ... && true);
    }

	static constexpr inline bool isConstSize() {
        return (TypeInfo<Types>::isConstSize() && ... && true);
	}
};

}

#endif /* ROLL_CPP_TYPES_AGGREGATETYPEINFOCOMMON_H_ */

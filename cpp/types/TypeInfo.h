#ifndef ROLL_CPP_TYPES_TYPEINFO_H_
#define ROLL_CPP_TYPES_TYPEINFO_H_

#include "common/Utility.h"
#include "common/CTStr.h"

namespace rpc {

template<class> struct TypeInfo;

/**
 * Checks - in compile time - that two types are compatible serialization-wise.
 */
template<class T, class U>
static constexpr bool isCompatible() {
	return TypeInfo<remove_cref_t<T>>::writeName(""_ctstr) == TypeInfo<remove_cref_t<U>>::writeName(""_ctstr);
}

}

#endif /* ROLL_CPP_TYPES_TYPEINFO_H_ */

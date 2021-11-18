#ifndef ROLL_CPP_TYPES_STDVECTORTYPEINFO_H_
#define ROLL_CPP_TYPES_STDVECTORTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <vector>

namespace rpc {

/**
 * Serialization rules for std::vector.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::vector<T>>: StlArrayBasedCollection<std::vector<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDVECTORTYPEINFO_H_ */

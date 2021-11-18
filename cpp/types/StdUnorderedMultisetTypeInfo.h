#ifndef ROLL_CPP_TYPES_STDUNORDEREDMULTISETTYPEINFO_H_
#define ROLL_CPP_TYPES_STDUNORDEREDMULTISETTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <unordered_set>

namespace rpc {

/**
 * Serialization rules for std::unordered_multiset.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::unordered_multiset<T>>: StlAssociativeCollection<std::unordered_multiset<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDUNORDEREDMULTISETTYPEINFO_H_ */

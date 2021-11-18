#ifndef ROLL_CPP_TYPES_STDUNORDEREDSETTYPEINFO_H_
#define ROLL_CPP_TYPES_STDUNORDEREDSETTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <unordered_set>

namespace rpc {

/**
 * Serialization rules for std::unordered_set.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::unordered_set<T>>: StlAssociativeCollection<std::unordered_set<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDUNORDEREDSETTYPEINFO_H_ */

#ifndef ROLL_CPP_TYPES_STDMULTISETTYPEINFO_H_
#define ROLL_CPP_TYPES_STDMULTISETTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <set>

namespace rpc {

/**
 * Serialization rules for std::multiset.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::multiset<T>>: StlAssociativeCollection<std::multiset<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDMULTISETTYPEINFO_H_ */

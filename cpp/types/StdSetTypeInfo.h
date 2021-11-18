#ifndef ROLL_CPP_TYPES_STDSETTYPEINFO_H_
#define ROLL_CPP_TYPES_STDSETTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <set>

namespace rpc
{

/**
 * Serialization rules for std::set.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::set<T>>: StlAssociativeCollection<std::set<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDSETTYPEINFO_H_ */

#ifndef ROLL_CPP_TYPES_STDDEQUETYPEINFO_H_
#define ROLL_CPP_TYPES_STDDEQUETYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <deque>

namespace rpc {

/**
 * Serialization rules for std::deque.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::deque<T>>: StlListBasedCollection<std::deque<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDDEQUETYPEINFO_H_ */

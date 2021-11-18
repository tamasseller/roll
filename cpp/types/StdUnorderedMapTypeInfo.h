#ifndef ROLL_CPP_TYPES_STDUNORDEREDMAPTYPEINFO_H_
#define ROLL_CPP_TYPES_STDUNORDEREDMAPTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <unordered_map>

namespace rpc {

/**
 * Serialization rules for std::unordered_map.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::unordered_map<K, V>>: StlAssociativeCollection<std::unordered_map<K, V>, std::pair<K, V>> {};

}

#endif /* ROLL_CPP_TYPES_STDUNORDEREDMAPTYPEINFO_H_ */

#ifndef ROLL_CPP_TYPES_STDUNORDEREDMULTIMAPTYPEINFO_H_
#define ROLL_CPP_TYPES_STDUNORDEREDMULTIMAPTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <unordered_map>

namespace rpc {

/**
 * Serialization rules for std::unordered_multimap.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::unordered_multimap<K, V>>: StlAssociativeCollection<std::unordered_multimap<K, V>, std::pair<K, V>> {};

}

#endif /* ROLL_CPP_TYPES_STDUNORDEREDMULTIMAPTYPEINFO_H_ */

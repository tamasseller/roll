#ifndef ROLL_CPP_TYPES_STDMAPTYPEINFO_H_
#define ROLL_CPP_TYPES_STDMAPTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <map>

namespace rpc {

/**
 * Serialization rules for std::map.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::map<K, V>>: StlAssociativeCollection<std::map<K, V>, std::pair<K, V>> {};

}

#endif /* ROLL_CPP_TYPES_STDMAPTYPEINFO_H_ */

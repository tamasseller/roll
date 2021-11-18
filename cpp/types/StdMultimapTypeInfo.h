#ifndef ROLL_CPP_TYPES_STDMULTIMAPTYPEINFO_H_
#define ROLL_CPP_TYPES_STDMULTIMAPTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <map>

namespace rpc {

/**
 * Serialization rules for std::multimap.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class K, class V> struct TypeInfo<std::multimap<K, V>>: StlAssociativeCollection<std::multimap<K, V>, std::pair<K, V>> {};

}

#endif /* ROLL_CPP_TYPES_STDMULTIMAPTYPEINFO_H_ */

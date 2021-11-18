#ifndef ROLL_CPP_TYPES_STDLISTTYPEINFO_H_
#define ROLL_CPP_TYPES_STDLISTTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <list>

namespace rpc {

/**
 * Serialization rules for std::list.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<class T> struct TypeInfo<std::list<T>>: StlListBasedCollection<std::list<T>, T> {};

}

#endif /* ROLL_CPP_TYPES_STDLISTTYPEINFO_H_ */

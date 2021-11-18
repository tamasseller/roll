#ifndef ROLL_CPP_TYPES_STDSTRINGTYPEINFO_H_
#define ROLL_CPP_TYPES_STDSTRINGTYPEINFO_H_

#include "StlTypeInfoCommon.h"

#include <string>

namespace rpc {

/**
 * Serialization rules for std::string.
 *
 * NOTE: see CollectionTypeBase for generic rules of collection serialization.
 */
template<> struct TypeInfo<std::string>: StlArrayBasedCollection<std::string, char> {};

}

#endif /* ROLL_CPP_TYPES_STDSTRINGTYPEINFO_H_ */

#ifndef ROLL_CPP_TYPES_COLLECTIONPLACEHOLDER_H_
#define ROLL_CPP_TYPES_COLLECTIONPLACEHOLDER_H_

#include "TypeInfo.h"

namespace rpc {

/**
 * Placeholder for a collection.
 *
 * Supports only signature generator (no read/write).
 * It can also be used as a base-class for serialization rule classes.
 */
template<class T> struct CollectionPlaceholder {
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return TypeInfo<T>::writeName(s << "[") << "]";
	}
};

/**
 * Serialization rules for the collection placeholder.
 *
 * Supports only signature generator (no read/write).
 */
template<class T> struct TypeInfo<CollectionPlaceholder<T>>
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return CollectionPlaceholder<T>::writeName(rpc::move(s));
	}
};

/**
 * Template specialization based global dependency injector for default collection container class.
 */
template<class Element, class Dummy = void> struct CollectionSelector {
	using Type = CollectionPlaceholder<Element>;
};

/**
 * Convenience wrapper for the global collection container dependency injector.
 */
template<class Element> using Many = typename CollectionSelector<Element>::Type;

}

#endif /* ROLL_CPP_TYPES_COLLECTIONPLACEHOLDER_H_ */

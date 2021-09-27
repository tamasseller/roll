#ifndef ROLL_CPP_RPCCOLLECTION_H_
#define ROLL_CPP_RPCCOLLECTION_H_

#include "RpcTypeInfo.h"

namespace rpc {

template<class T> struct CollectionPlaceholder {
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return TypeInfo<T>::writeName(s << "[") << "]";
	}
};

template<class T> struct TypeInfo<CollectionPlaceholder<T>>
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) {
		return CollectionPlaceholder<T>::writeName(rpc::move(s));
	}
};

template<class Element, class Dummy = void> struct CollectionSelector {
	using Type = CollectionPlaceholder<Element>;
};

template<class Element> using Many = typename CollectionSelector<Element>::Type;

}

#endif /* ROLL_CPP_RPCCOLLECTION_H_ */

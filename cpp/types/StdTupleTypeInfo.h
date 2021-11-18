#ifndef ROLL_CPP_TYPES_STDTUPLETYPEINFO_H_
#define ROLL_CPP_TYPES_STDTUPLETYPEINFO_H_

#include "AggregateTypeInfoCommon.h"

#include <tuple>

namespace rpc {

/**
 * Serialization rules for std::tuple
 *
 * NOTE: see AggregateTypeBase for generic rules of aggregate serialization.
 */
template<class... Types> struct TypeInfo<std::tuple<Types...>>: AggregateTypeBase<Types...>
{
   	static constexpr inline size_t size(const std::tuple<Types...>& v)  {
        return std::apply([] (auto&&... x) { return (TypeInfo<Types>::size(x) + ... + 0); }, v);
    }

	template<class S> static inline bool write(S& s, const std::tuple<Types...>& v) {
        return std::apply([&s] (auto&&... x) { return (TypeInfo<Types>::write(s, x) && ... && true); }, v);
    }

	template<class S> static inline bool read(S& s, std::tuple<Types...>& v) {
        return std::apply([&s] (auto&&... x) { return (TypeInfo<Types>::read(s, x) && ... && true); }, v);
    }
};

}

#endif /* ROLL_CPP_TYPES_STDTUPLETYPEINFO_H_ */

#ifndef ROLL_CPP_TYPES_DEREFERENCETYPEINFO_H_
#define ROLL_CPP_TYPES_DEREFERENCETYPEINFO_H_

#include <cstddef>

namespace rpc
{

template<class Info> struct DereferenceTypeInfo
{
	template<class S> static constexpr inline decltype(auto) writeName(S&& s) { return Info::writeName(s); }
	template<class S, class Type> static inline bool write(S& s, Type v) { return Info::write(s, *v); }
	template<class S, class Type> static inline bool read(S& s, Type v) { return Info::read(s, *v); }
	template<class Type> static constexpr inline size_t size(Type v) { return Info::size(*v); }
	template<class S> static inline bool skip(S& s) { return Info::skip(s); }
	static constexpr inline bool isConstSize() { return Info::isConstSize(); }
};

}

#endif /* ROLL_CPP_TYPES_DEREFERENCETYPEINFO_H_ */

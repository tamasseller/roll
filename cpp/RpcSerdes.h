#ifndef COMMON_RPCSERDES_H_
#define COMMON_RPCSERDES_H_

#include "RpcTypeInfo.h"

#include <utility>
#include <type_traits>

namespace rpc {

namespace detail
{
	template<class... Args> struct Stripped
	{
		template<class C>
		static inline constexpr void call(C&& c, Args&&... args, bool &) {
			c(std::forward<Args>(args)...);
		}

		template<class Next> using Append = Stripped<Args..., Next>;
	};

	template<class... Args> struct StripLast;
	template<> struct StripLast<bool &>  {
		using Result = Stripped<>;
	};

	template<class First, class... Rest> struct StripLast<First, Rest...> {
		using Result = typename StripLast<Rest...>::Result::template Append<First>;
	};

	struct CallHelper
	{
		template<class C, class... Types>
		inline constexpr CallHelper(C&& c, Types&&... args) 
		{
			bool ok = (args, ...);
			if(ok)
				StripLast<Types...>::Result::template call<C>(std::forward<C>(c), std::forward<Types>(args)...);				
		}
	};
	
	template<class T, class S>
	static inline T readNext(S& s, bool &ok) 
	{
		T v;

		if(ok)
		{
			if(!TypeInfo<T>::read(s, v))
				ok = false;
		}
		
		return v;
	}
}

namespace detail
{
	template<class C>
	static constexpr inline size_t getSize(C&& c) {
		using Plain = typename std::remove_const_t<std::remove_reference_t<C>>;
		return TypeInfo<Plain>::size(std::forward<C>(c));
	}

	template<class S, class C>
	static constexpr inline size_t writeEntry(S& s, C&& c) {
		using Plain = typename std::remove_const_t<std::remove_reference_t<C>>;
		return TypeInfo<Plain>::write(s, std::forward<C>(c));
	}
}

template<class... Args>
size_t determineSize(Args&&... args) {
	return (detail::getSize(std::forward<Args>(args)) + ... + 0);
}

template<class... Args, class S>
bool serialize(S& s, Args&&... args)
{
	return (detail::writeEntry(s, std::forward<Args>(args)) && ... && true);
}

template<class... Args, class C, class S>
static inline auto deserialize(S& s, C&& c)
{
	bool ok = true;
	detail::CallHelper{std::forward<C>(c), detail::readNext<Args>(s, ok)..., ok};
	return ok;
}

}

#endif /* COMMON_RPCSERDES_H_ */

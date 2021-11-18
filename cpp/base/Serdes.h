#ifndef _RPCSERDES_H_
#define _RPCSERDES_H_

#include "common/Utility.h"
#include "common/Errors.h"

#include "types/TypeInfo.h"

namespace rpc {

namespace detail
{
	template<class> struct RetvalHelper;
	
	template<> struct RetvalHelper<void>
	{
		template<class V, class... A>
		static inline constexpr const char* execute(V&& c, A&&... a) {
			return c(rpc::forward<A>(a)...), nullptr;
		}
	};

	template<> struct RetvalHelper<const char*>
	{
		template<class V, class... A>
		static inline constexpr const char* execute(V&& c, A&&... a) {
			return c(rpc::forward<A>(a)...);
		}
	};

	template<class... Args> struct Stripped
	{
		template<class C>
		static inline constexpr auto call(C&& c, Args&&... args, bool &) {
			return RetvalHelper<decltype(c(rpc::forward<Args>(args)...))>::execute(rpc::forward<C>(c), rpc::forward<Args>(args)...);
		}

		template<class Next> using Prepend = Stripped<Next, Args...>;
	};

	template<class... Args> struct StripLast;
	template<> struct StripLast<bool &>  {
		using Result = Stripped<>;
	};

	template<class First, class... Rest> struct StripLast<First, Rest...> {
		using Result = typename StripLast<Rest...>::Result::template Prepend<First>;
	};

	/**
	 * Helper that ensures correct order of evaluation of arguments via the "brace-enclosed
	 * comma-separated list of initalizers" sequencing rule.
	 */
	struct CallHelper
	{
		const char* result = Errors::wrongMethodRequest;

		template<class C, class... Types>
		inline constexpr CallHelper(C&& c, Types&&... args) 
		{
			bool ok = (args, ...);

			if(ok)
			{
				result = StripLast<Types...>::Result::template call<C>(rpc::forward<C>(c), rpc::forward<Types>(args)...);
			}
			else
			{
				result = Errors::messageFormatError;
			}
		}
	};
	
	template<class T>
	struct ReadNext
	{
		T v;

		template<class S>
		inline ReadNext(S& s, bool &ok)
		{
			if(ok)
			{
				if(!TypeInfo<T>::read(s, v))
				{
					ok = false;
				}
			}
		}
	};

	template<class Arg>
	static constexpr inline size_t getSize(const Arg& c) {
		return TypeInfo<remove_cref_t<Arg>>::size(c);
	}

	template<class Arg, class S>
	static constexpr inline size_t writeEntry(S& s, Arg&& c) {
		return TypeInfo<remove_cref_t<Arg>>::write(s, rpc::forward<Arg>(c));
	}
}

/**
 * Helper method that determines the serialized size of the values passed 
 * in args using the TypeInfo template class.
 */
template<class... Args>
size_t determineSize(const Args&... args) {
	return (detail::getSize(args) + ... + 0);
}

/**
 * Helper method that serializes the values passed in args into a stream 
 * using the TypeInfo template class.
 */
template<class... Args, class S>
bool serialize(S& s, Args&&... args)
{
	return (detail::writeEntry(s, rpc::forward<Args>(args)) && ... && true);
}

/**
 * Helper method that deserializes values from a stream and calls 
 * a functor using them as arguments using the TypeInfo template class.
 */
template<class... Args, class... ExtraArgs, class C, class S>
static inline const char* deserialize(S& s, C&& c, ExtraArgs&&... extraArgs)
{
	bool ok = true;

	//
	//  In list-initialization, every value computation and side effect of a given
	//  initializer clause is sequenced before every value computation and side effect
	//  associated with any initializer clause that follows it in the brace-enclosed
	//  comma-separated list of initalizers.
	//
	return detail::CallHelper{rpc::forward<C>(c), rpc::forward<ExtraArgs>(extraArgs)..., detail::ReadNext<remove_cref_t<Args>>(s, ok).v..., ok}.result;
}

}

#endif /* _RPCSERDES_H_ */

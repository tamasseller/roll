#ifndef _RPCSERDES_H_
#define _RPCSERDES_H_

#include "RpcTypeInfo.h"
#include "RpcUtility.h"
#include "RpcErrors.h"

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

	struct CallHelper
	{
		const char* result = Errors::wrongMethodRequest;

		template<class C, class... Types>
		inline constexpr CallHelper(C&& c, Types&&... args) 
		{
			bool ok = (args, ...);
			if(ok)
				result = StripLast<Types...>::Result::template call<C>(rpc::forward<C>(c), rpc::forward<Types>(args)...);
			else
				result = Errors::messageFormatError;
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

	template<class NominalArgs, class ActualArg>
	static constexpr inline size_t getSize(const ActualArg& c) {
		return TypeInfo<NominalArgs>::size(c);
	}

	template<class NominalArgs, class ActualArg, class S>
	static constexpr inline size_t writeEntry(S& s, ActualArg&& c) {
		return TypeInfo<NominalArgs>::write(s, rpc::forward<ActualArg>(c));
	}
}

/**
 * Helper method that determines the serialized size of the values passed 
 * in args using the TypeInfo template class.
 */
template<class... NominalArgs, class... ActualArgs>
size_t determineSize(const ActualArgs&... args) {
	return (detail::getSize<NominalArgs>(args) + ... + 0);
}

/**
 * Helper method that serializes the values passed in args into a stream 
 * using the TypeInfo template class.
 */
template<class... NominalArgs, class... ActualArgs, class S>
bool serialize(S& s, ActualArgs&&... args)
{
	return (detail::writeEntry<NominalArgs>(s, rpc::forward<ActualArgs>(args)) && ... && true);
}

/**
 * Helper method that deserializes values from a stream and calls 
 * a functor using them as arguments using the TypeInfo template class.
 */
template<class... Args, class... ExtraArgs, class C, class S>
static inline const char* deserialize(S& s, C&& c, ExtraArgs&&... extraArgs)
{
	bool ok = true;
	return detail::CallHelper{rpc::forward<C>(c), rpc::forward<ExtraArgs>(extraArgs)..., detail::readNext<Args>(s, ok)..., ok}.result;
}

}

#endif /* _RPCSERDES_H_ */

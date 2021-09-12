#ifndef _RPCUTILITY_H_
#define _RPCUTILITY_H_

namespace rpc {

namespace detail  
{
    struct false_type { static constexpr auto value = false; };
    struct true_type { static constexpr auto value = true; };
    template<class> struct is_lvalue_reference: public false_type { };
    template<class T> struct is_lvalue_reference<T&>: public true_type { };

    template<int n, class> struct nth_argument;

    template<class C, class A, class... As> struct nth_argument<0, void(C::*)(A, As...)> { using T = A; };
    template<class C, class A, class... As> struct nth_argument<0, void(C::*)(A, As...) const> { using T = A; };

    template<int n, class C, class A, class... As> struct nth_argument<n, void(C::*)(A, As...)> { using T = typename nth_argument<n - 1, void(C::*)(A, As...)>::T; };
    template<int n, class C, class A, class... As> struct nth_argument<n, void(C::*)(A, As...) const> { using T = typename nth_argument<n - 1, void(C::*)(A, As...)>::T; };
}

/**
 * Same as std::remove_reference, re-implemented for dependency purposes.
 */
template<class T> struct remove_reference { typedef T type; };
template<class T> struct remove_reference<T&> { typedef T type; };
template<class T> struct remove_reference<T&&> { typedef T type; };

/**
 * Same as std::remove_reference_t, re-implemented for dependency purposes.
 */
template<class T> using remove_reference_t = typename remove_reference<T>::type;

/**
 * Same as std::remove_const, re-implemented for dependency purposes.
 */
template<class T> struct remove_const { using type = T; };
template<class T> struct remove_const<const T> { using type = T;  };

/**
 * Same as std::remove_const_t, re-implemented for dependency purposes.
 */
template<class T> using remove_const_t = typename remove_const<T>::type;

template<class T> using remove_cref_t = remove_const_t<remove_reference_t<T>>;


/**
 * Same as std::forward, re-implemented for dependency purposes.
 */
template<typename T>
static inline constexpr T&& forward(remove_reference_t<T>& t) {
    return static_cast<T&&>(t);
}

/**
 * Same as std::forward, re-implemented for dependency purposes.
 */
template<typename T>
static inline constexpr T&& forward(remove_reference_t<T>&& t)
{
    static_assert(!detail::is_lvalue_reference<T>::value, "template argument substituting T is an lvalue reference type");
    return static_cast<T&&>(t);
}

/**
 * Same as std::move, re-implemented for dependency purposes.
 */
template<typename T>
constexpr remove_reference_t<T>&& move(T&& t) {
    return static_cast<remove_reference_t<T>&&>(t);
}

template<int n, class C> using Arg = typename detail::nth_argument<n, decltype(&C::operator())>::T;

}

template <class T, T... cs>
static inline auto operator "" _nt() -> const T (&)[sizeof...(cs)] {
	static constexpr const T str[] = {cs...};
	return str;
}

#endif /* _RPCUTILITY_H_ */

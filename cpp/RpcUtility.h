#ifndef _RPCUTILITY_H_
#define _RPCUTILITY_H_

namespace rpc {

namespace detail  
{
    struct false_type { static constexpr auto value = false; };
    struct true_type { static constexpr auto value = true; };
    template<class> struct is_lvalue_reference: public false_type { };
    template<class T> struct is_lvalue_reference<T&>: public true_type { };
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

}

#endif /* _RPCUTILITY_H_ */

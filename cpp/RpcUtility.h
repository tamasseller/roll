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

template<class T> struct remove_reference { typedef T type; };
template<class T> struct remove_reference<T&> { typedef T type; };
template<class T> struct remove_reference<T&&> { typedef T type; };
template<class T> using remove_reference_t = typename remove_reference<T>::type;

template<class T> struct remove_const { using type = T; };
template<class T> struct remove_const<const T> { using type = T;  };
template<class T> using remove_const_t = typename remove_const<T>::type;


template<typename T>
static inline constexpr T&& forward(remove_reference_t<T>& t) {
    return static_cast<T&&>(t);
}

template<typename T>
static inline constexpr T&& forward(remove_reference_t<T>&& t)
{
    static_assert(!detail::is_lvalue_reference<T>::value, "template argument substituting T is an lvalue reference type");
    return static_cast<T&&>(t);
}

template<typename T>
constexpr remove_reference_t<T>&& move(T&& t) {
    return static_cast<remove_reference_t<T>&&>(t);
}

}

#endif /* _RPCUTILITY_H_ */

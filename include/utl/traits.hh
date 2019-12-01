/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */

#ifndef UTL_TRAITS_HH_
#define UTL_TRAITS_HH_

namespace utl {

    template <bool, typename T = void>
    struct enable_if {};

    template <typename T>
    struct enable_if<true, T> { using type = T; };

    template <bool B, typename T>
    using enable_if_t = typename enable_if<B,T>::type;

    struct true_type { static constexpr bool value = true; };
    struct false_type { static constexpr bool value = false; };

    template <typename...>
    using void_t = void;

    template <typename T, T v>
    struct integral_constant
    {
        static constexpr T value = v;
        using value_type = T;
    };

    template <typename T1, typename T2>
    struct is_same : false_type {};

    template <typename T>
    struct is_same<T,T> : true_type {};

    template <typename T1, typename T2>
    static constexpr bool is_same_v = is_same<T1,T2>::value;

    template <bool B, typename T, typename F>
    struct conditional { using type = T; };

    template <typename T, typename F>
    struct conditional<false,T,F> { using type = F; };

    template <bool B, typename T, typename F>
    using conditional_t = typename conditional<B,T,F>::type;

    template <typename T>
    struct is_const : false_type {};

    template <typename T>
    struct is_const<const T> : true_type {};

    template <typename T>
    struct is_const<const T&> : true_type {};

    template <typename T>
    struct is_enum {
        static constexpr bool value = __is_enum(T);
    };

    template <class T, bool = is_enum<T>::value> struct _Underlying_type {};
    template <class T> struct _Underlying_type<T, true> { using type = __underlying_type(T); };
    template <class T> struct underlying_type : _Underlying_type<T> { };

    template <typename T>
    struct remove_reference { using type = T; };

    template <typename T>
    struct remove_reference<T&> { using type = T; };

    template <typename T>
    struct remove_reference<T&&> { using type = T; };

    template <typename T, typename... Ts>
    struct index_of;

    template <typename T, typename... Ts>
    struct index_of<T,T,Ts...> : integral_constant<size_t,0> {};

    template <typename T, typename S, typename... Ts>
    struct index_of<T,S,Ts...> :
        integral_constant<size_t,1+index_of<T,Ts...>::value> {};

    template <size_t I, typename... Ts>
    struct type_of;

    template <size_t I, typename T, typename... Ts>
    struct type_of<I,T,Ts...> : type_of<I-1,Ts...> {};

    template <typename T, typename... Ts>
    struct type_of<0,T,Ts...> { using type = T; };

    // template <typename T, typename... Ts>
    // struct contains;

    // template <typename T>
    // struct contains<T> { static constexpr bool value = false; };

    // template <typename T, typename S, typename... Ts>
    // struct contains<T,S,Ts...>
    // {
    //     static constexpr bool value =
    //         is_same<T,S>::value or contains<T,Ts...>::value;
    // };

    template <typename... Ts>
    struct max_width_t;

    template <typename T>
    struct max_width_t<T> { using type = T; };

    template <typename T, typename S, typename... Ts>
    struct max_width_t<T,S,Ts...>
    {
        using cond_t = typename conditional<(sizeof(T)>=sizeof(S)),T,S>::type;
        using type = typename max_width_t<cond_t,Ts...>::type;
    };

    template <typename... Ts>
    struct max_align_t;

    template <typename T>
    struct max_align_t<T> { using type = T; };

    template <typename T, typename S, typename... Ts>
    struct max_align_t<T,S,Ts...>
    {
        using cond_t = typename conditional<(alignof(T)>=alignof(S)),T,S>::type;
        using type = typename max_align_t<cond_t,Ts...>::type;
    };

} // namespace utl
#endif // UTL_TRAITS_HH_

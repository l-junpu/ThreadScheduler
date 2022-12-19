namespace jpd
{
    /*
        Type Decay
    */
    template<typename T>
    using BaseType = std::remove_const_t< std::remove_pointer_t< std::decay_t<T> >>;


    /*
        Function Traits
    */
    template <class F>
    struct Traits;

    template <typename ReturnType, typename... T_Args>
    struct FuncTraits
    {
        using Return_T                  =  ReturnType;
        using Args_T                    =  std::tuple<T_Args...>;
        constexpr static size_t Args_C  =  sizeof...(T_Args);
    };

    // Normal Function
    template <typename R, typename... T_Args>           struct Traits< R(T_Args...) >                      : FuncTraits<R, T_Args...> {};
    template <typename R, typename... T_Args>           struct Traits< R(T_Args...) noexcept >             : FuncTraits<R, T_Args...> {};
    // Function Pointer
    template <typename R, typename... T_Args>           struct Traits< R(*)(T_Args...) >                   : FuncTraits<R, T_Args...> {};
    template <typename R, typename... T_Args>           struct Traits< R(*)(T_Args...) noexcept >          : FuncTraits<R, T_Args...> {};
    // Member Function
    template <class T, typename R, typename... T_Args > struct Traits< R(T::*)(T_Args...) >                : FuncTraits<R, T_Args...> {};
    template <class T, typename R, typename... T_Args > struct Traits< R(T::*)(T_Args...) noexcept >       : FuncTraits<R, T_Args...> {};
    template <class T, typename R, typename... T_Args > struct Traits< R(T::*)(T_Args...) const >          : FuncTraits<R, T_Args...> {};
    template <class T, typename R, typename... T_Args > struct Traits< R(T::*)(T_Args...) const noexcept > : FuncTraits<R, T_Args...> {};

    // Decay
    template<class T> struct Traits            : Traits<decltype(&T::operator())> { using Class_T = T; };
    template<class T> struct Traits<T&>        : Traits<T> {};
    template<class T> struct Traits<const T&>  : Traits<T> {};
    template<class T> struct Traits<T&&>       : Traits<T> {};
    template<class T> struct Traits<const T&&> : Traits<T> {};
    template<class T> struct Traits<T*>        : Traits<T> {};
    template<class T> struct Traits<const T*>  : Traits<T> {};



    template< typename T_TUPLE>
    constexpr auto null_tuple_v = static_cast<T_TUPLE*>(0);
    
    

    // Extract Tuple Types
    template <int N, typename... Ts>
    struct Extract;

    template <int N, typename T, typename... Ts>
    struct Extract<N, std::tuple<T, Ts...>>
    {
        using Type = typename Extract<N - 1, std::tuple<Ts...>>::type;
    };

    template <typename T, typename... Ts>
    struct Extract<0, std::tuple<T, Ts...>>
    {
        using Type = T;
    };
}
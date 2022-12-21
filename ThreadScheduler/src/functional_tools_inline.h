#define REF(x)                 \
    std::ref(x)

#define BEGIN_SCOPE_LOCK(x)    \
{                              \
    std::scoped_lock __sl(x);
#define END_SCOPE_LOCK()       \
}


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


    /*
        Print T_Args Types
    */
    template <typename... Args>
    void PrintTypes(std::ostream& out, Args&&... args)
    {
        (( out << "Type: " << typeid(args).name() << " | "), ...);
        out << "\n";
    }
    template <typename... Args>
    void PrintTypes(std::ostream& out, std::tuple<Args...>&)
    {
        ((out << "Type: " << typeid(Args).name() << " | "), ...);
        out << "\n";
    }
    template <typename... Args>
    void PrintTypes(std::ostream& out)
    {
        ((out << "Type: " << typeid(Args).name() << " | "), ...);
        out << "\n";
    }

    /*
        Concepts
    */
    template <typename ReturnType>
    concept IsVoid_T = std::is_same_v<ReturnType, void>;

    template <typename ReturnType>
    concept NotVoid_T = !(std::is_same_v<ReturnType, void>);



    //template <typename... T_Args, typename... T_Values, typename T_Concat_Tuple = void>
    //inline std::tuple<T_Args...> ReturnValidType(std::tuple<T_Args...>* Args, std::tuple<T_Values...>* Values, T_Concat_Tuple* Output, const size_t Index)
    //{
    //    using FuncParamType  = std::tuple<T_Args...>;
    //    using InputParamType = std::tuple<T_Values...>;

    //    using ExtractedType = Extract<Index, FuncParamType>::Type;
    //    using InputType     = Extract<Index, InputParamType>::Type;

    //    // Index >= 1
    //    if (Output)
    //    {
    //        auto CatTuple = std::tuple_cat(*Output, std::is_lvalue_reference_v<ExtractedType> ? std::ref(std::get<Index>(*Values))
    //                                                                                          : std::forward<InputType>(std::get<Index>(Values)) );

    //        if (Index + 1 == sizeof...(T_Args))
    //        {
    //            PrintTypes(std::cout, CatTuple);
    //            return std::move(CatTuple);
    //        }
    //        else
    //        {
    //            return ReturnValidType(Args, Values, &CatTuple, Index+1);
    //        }
    //    }
    //    // Index == 0
    //    else
    //    {
    //        assert(Index == 0);

    //        auto BaseTuple = std::make_tuple<ExtractedType>( std::is_lvalue_reference_v<ExtractedType> ? std::ref(std::get<Index>(*Values))
    //                                                                                                   : std::forward<InputType>(std::get<Index>(Values)) );
    //        return ReturnValidType(Args, Values, &BaseTuple, Index+1);
    //    }
    //}

    //template <typename... T_Args, typename... T_Values>
    //inline std::tuple<T_Args...> ReturnValidType(std::tuple<T_Args...>* Args, std::tuple<T_Values...>* Values)
    //{
    //    assert( sizeof...(T_Args) == sizeof...(T_Values) );
    //    int* a = nullptr;
    //    return ReturnValidType(Args, Values, a, 0);
    //}
}
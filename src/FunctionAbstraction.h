#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <optional>
#include <array>
#include <functional>

template<typename T>
concept Printable = requires(T t) {
    { t.Print() } -> std::same_as<void>;
};

enum LibEnum{
    LIB_LAB = 0,
    LIB_D9DX = 1,


    LibEnum_SIZE,
};

template <typename T>
struct function_traits;

template <typename Result, typename... Args>
struct function_traits<Result(*)(Args...)> {
    using result_type = Result;
    using args_tuple  = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};
template <typename Func>
using first_arg_t = std::tuple_element_t<0, typename function_traits<std::decay_t<Func>>::args_tuple>;


template<typename Result, auto... Funcs>
struct FunctionAbstraction{
    static constexpr auto funcs = std::tuple{Funcs...};
    
    template <std::size_t I, typename... Args>
    static decltype(auto) Call(Args&&... args) {
        constexpr auto F = std::get<I>(funcs);
        if constexpr (F == nullptr) {
                return;
        } 
        else {
            using FuncResult = std::invoke_result_t<decltype(F), Args...>;
            if constexpr (std::is_same_v<FuncResult, Result> || std::is_convertible_v<FuncResult, Result>) {
                if constexpr (std::is_void_v<Result>) {
                    std::get<I>(funcs)(std::forward<Args>(args)...);
                    return;
                } else {
                    return std::get<I>(funcs)(std::forward<Args>(args)...);
                }
            }
            else{
                using FirstParamType = first_arg_t<decltype(F)>;
                static_assert(std::is_lvalue_reference_v<FirstParamType>, "if the first param isn't the out param, the function needs special handling");
                using RetType = std::remove_reference_t<FirstParamType>;
                static_assert(std::is_same_v<RetType, Result> || std::is_convertible_v<RetType, Result>, "First param should match the return type");
                RetType ret{};
                std::get<I>(funcs)(ret, std::forward<Args>(args)...);
                return ret;
            }
        }
    }

    template<std::size_t... Is, typename... Args>
    static std::array<std::optional<Result>, LibEnum_SIZE>
    Call_All_Impl(std::index_sequence<Is...>, Args&&... args) {
        // pack expansion calls Call<0>, Call<1>, ... and stores in array
        return { Call<Is>(std::forward<Args>(args)...)... }; 
    }

    template<typename... Args>
    static std::array<std::optional<Result>, LibEnum_SIZE> Call_All(Args&&... args) {
        return Call_All_Impl(std::make_index_sequence<LibEnum_SIZE>{}, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void Call_All_Print(Args&&... args) {
        static_assert(Printable<Result>, "attempting to print with a non-printable type");
        auto results = Call_All_Impl(std::make_index_sequence<LibEnum_SIZE>{}, std::forward<Args>(args)...);
        for(auto& ret : results){
            if(ret.has_value()){
                ret->Print();
            }
        }
    }
    template<typename PrintFunc, typename... Args>
    static void Call_All_PrintFunc(PrintFunc&& printFunc, Args&&... args) {
        auto results = Call_All_Impl(std::make_index_sequence<LibEnum_SIZE>{}, std::forward<Args>(args)...);
        for(auto& ret : results){
            if(ret.has_value()){
                printFunc(*ret);
            }
        }
    }
};  
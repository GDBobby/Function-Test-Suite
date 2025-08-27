#pragma once

#include <random>
#include <concepts>
#include <type_traits>
#include <tuple>

#include "Supporting.h"

namespace BATS {

    template<typename T>
    struct Function_Traits;

    template<typename R, typename... Args>
    struct Function_Traits<R(*)(Args...)> {
        using args_tuple = std::tuple<Args...>;
        using Return_Type = R;
        static constexpr bool is_member_function = false;

        template<typename T>
        static args_tuple random_args(T lowerBound, T upperBound) {
            return args_tuple{ random_value<std::decay_t<Args>>(static_cast<std::decay_t<Args>>(lowerBound), static_cast<std::decay_t<Args>>(upperBound))... };
        }
    };

    template<typename R, typename C, typename... Args>
    struct Function_Traits<R(C::*)(Args...)> {
        using args_tuple = std::tuple<Args...>;
        using Class_Type = C;
        using Return_Type = R;
        static constexpr bool is_member_function = true;

        template<typename T>
        static args_tuple random_args(T lowerBound, T upperBound) {
            return args_tuple{
                random_value<std::decay_t<Args>>(
                    static_cast<std::decay_t<Args>>(lowerBound),
                    static_cast<std::decay_t<Args>>(upperBound)
                )...
            };
        }
    };
    template<typename R, typename C, typename... Args>
    struct Function_Traits<R(C::*)(Args...) const> {
        using args_tuple = std::tuple<Args...>;
		using Class_Type = C;
        using Return_Type = R;
        static constexpr bool is_member_function = true;

        template<typename T>
        static args_tuple random_args(T lowerBound, T upperBound) {
            return args_tuple{
                random_value<std::decay_t<Args>>(
                    static_cast<std::decay_t<Args>>(lowerBound),
                    static_cast<std::decay_t<Args>>(upperBound)
                )...
            };
        }
    };

    template <typename Func, bool HasArgs =
        (std::tuple_size_v<typename Function_Traits<std::decay_t<Func>>::args_tuple> > 0)>
        struct first_arg_helper;

    template <typename Func>
    struct first_arg_helper<Func, true> {
        using type = std::tuple_element_t<
            0, typename Function_Traits<std::decay_t<Func>>::args_tuple>;
    };

    template <typename Func>
    struct first_arg_helper<Func, false> {
        using type = void;
    };

    template <typename Func>
    using first_arg_t = typename first_arg_helper<Func>::type;
}
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
        using return_type = R;
        using args_tuple = std::tuple<Args...>;

        template<typename T>
        static args_tuple random_args(T lowerBound, T upperBound) {
            return args_tuple{ random_value<std::decay_t<Args>>(static_cast<std::decay_t<Args>>(lowerBound), static_cast<std::decay_t<Args>>(upperBound))... };
        }
    };

    template <typename Func>
    using first_arg_t = std::tuple_element_t<0, typename Function_Traits<std::decay_t<Func>>::args_tuple>;

}
#pragma once

#include <random>
#include <concepts>
#include <type_traits>
#include <tuple>

namespace BATS {

    template<typename T, typename U>
    T random_value(U lowerBounds, U upperBounds) {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(lowerBounds, upperBounds);
            return dist(gen);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(lowerBounds, upperBounds);
            return dist(gen);
        }
        else if constexpr (requires { T::GenerateRandom(lowerBounds, upperBounds); }) {
			return T::GenerateRandom(lowerBounds, upperBounds);
        }
        else {
            return U(0);
            //static_assert(std::is_fundamental_v<T>, "Unsupported type");
        }
    }

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

    template<typename U, typename... Args>
    std::tuple<Args...> GenerateRandomTuple(U lowerBound, U upperBound) {
        return std::tuple<Args...>{ random_value<Args>(lowerBound, upperBound)... };
    }
}
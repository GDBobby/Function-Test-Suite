#pragma once

#include <random>
#include <tuple>
#include <array>
#include <concepts>
#include <type_traits>

namespace BATS {
    template<typename T>
    inline void MAX_EQUALS(T& a, T const& b) {
        a = a > b ? a : b;
    }
    template<typename T>
    inline void MIN_EQUALS(T& a, T const& b) {
        a = a < b ? a : b;
    }

    static uint64_t un_optimizer = 0;
    template <class T>
    inline void DoNotOptimize(T const& value) {
        un_optimizer ^= reinterpret_cast<uint64_t>(&value);
    }

    template <class T>
    inline void DoNotOptimize(T& value) {
        un_optimizer ^= reinterpret_cast<uint64_t>(&value);
    }

    void Noop() {
        int noop = 0;
        DoNotOptimize(noop);
    }

    template<typename T>
    concept Printable = requires(T t) {
        { t.Print() } -> std::same_as<void>;
    };




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


    template<typename U, typename... Args>
    std::tuple<Args...> GenerateRandomTuple(U lowerBound, U upperBound) {
        return std::tuple<Args...>{ random_value<Args>(lowerBound, upperBound)... };
    }
}
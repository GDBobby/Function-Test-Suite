#pragma once

#include "../BATS/Support/FunctionTraits.h"

#include <cstdint>
#include <cstdio>
#include <variant>
#include <type_traits>
#include <concepts>

template <typename T, typename MainType, typename... Types>
concept InTypes = std::same_as<T, MainType> || (std::same_as<T, Types> || ...);

template <typename MainType, typename... Types>
struct TypeAbstraction{
    MainType value;

    static TypeAbstraction GenerateRandom(float lowerBound, float upperBound) {
        TypeAbstraction ret{};
		constexpr std::size_t float_count = sizeof(TypeAbstraction) / sizeof(float);
        float buffer[float_count];
        for (uint8_t i = 0; i < float_count; i++) {
            buffer[i] = BATS::random_value<float>(lowerBound, upperBound);
        }
		memcpy(&ret, buffer, sizeof(TypeAbstraction));
        return ret;
    }

    template <typename T>
    requires InTypes<T, MainType, Types...>
    TypeAbstraction(T const& v) {
        if constexpr (std::same_as<T, MainType>) {
            value = v;
        }
        else {
            memcpy(&value, &v, sizeof(MainType));
        }
    }
    
	TypeAbstraction() = default;

    template <typename T>
    requires InTypes<T, MainType, Types...>
    operator T&() {
        if constexpr (std::same_as<T, MainType>) {
            return value;
        }
        else {
            return *reinterpret_cast<T*>(&value);
        }
    }

    template <typename T>
    requires InTypes<T, MainType,Types...>
    operator const T&() const {
        if constexpr (std::same_as<T, MainType>) {
            return value;
        }
        else {
            return *reinterpret_cast<const T*>(&value);
        }
    }

    template <typename T>
        requires InTypes<T, MainType, Types...>
    operator T* () {
        if constexpr (std::same_as<T, MainType>) {
            return &value;
        }
        else {
            return reinterpret_cast<T*>(&value);
        }
    }

    template <typename T>
        requires InTypes<T, MainType, Types...>
    operator const T* () const {
        if constexpr (std::same_as<T, MainType>) {
            return &value;
        }
        else {
            return reinterpret_cast<const T*>(&value);
        }
    }
};
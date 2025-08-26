#pragma once

#include "../BATS/Support/FunctionTraits.h"

#include <cstdint>
#include <cstdio>
#include <variant>
#include <type_traits>
#include <concepts>

template <typename T, typename LabType, typename... Types>
concept InTypes = std::same_as<T, LabType> || (std::same_as<T, Types> || ...);

template <typename LabType, typename... Types>
struct TypeAbstraction{
    LabType value;


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
    requires InTypes<T, LabType, Types...>
    TypeAbstraction(T const& v) {
        if constexpr (std::same_as<T, LabType>) {
            value = v;
        }
        else {
            memcpy(&value, &v, sizeof(LabType));
        }
    }
    
	TypeAbstraction() = default;

    template <typename T>
    requires InTypes<T, LabType, Types...>
    operator T&() {
        if constexpr (std::same_as<T, LabType>) {
            return value;
        }
        else {
            return *reinterpret_cast<T*>(&value);
        }
    }

    template <typename T>
    requires InTypes<T, LabType,Types...>
    operator const T&() const {
        if constexpr (std::same_as<T, LabType>) {
            return value;
        }
        else {
            return *reinterpret_cast<T*>(&value);
        }
    }
};
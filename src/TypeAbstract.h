#pragma once

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
#pragma once

#include <cstdint>
#include <array>
#include <chrono>
#include <tuple>

namespace BATS {
    template<uint16_t Count>
    struct BenchmarkResults {
        std::array<std::chrono::nanoseconds, Count> total;
        std::array<std::chrono::nanoseconds, Count> max;
        std::array<std::chrono::nanoseconds, Count> min;

        const uint64_t iteration_count;

        BenchmarkResults(uint64_t iteration_count) : iteration_count(iteration_count) {
            total.fill(std::chrono::nanoseconds(0));
            max.fill(std::chrono::nanoseconds(0));
            min.fill(std::chrono::nanoseconds::max());
        }
    };

    template<uint16_t Count, typename Result, typename... Args>
    struct AccuracyResult {
        std::tuple<Args...> args;
        std::array<Result, Count> mismatched{};

        AccuracyResult(std::array<Result, Count> const& results, std::tuple<Args...>&& args) : args{ args }, mismatched{ results } {}
    };
}
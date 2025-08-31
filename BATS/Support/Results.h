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

        static void Print(std::string const& name, std::vector<AccuracyResult>& results, const float epsilon) {
            
            for (uint32_t i = 0; i < results.size(); i++) {
                bool biggerThanEpsilon = false;
                constexpr std::size_t resultSize = sizeof(results[i].mismatched[0]) / sizeof(float);
                for (uint32_t j = 1; j < results[i].mismatched.size(); j++) {
                    for (uint32_t k = 0; k < resultSize; k++) {
                        const float diff = reinterpret_cast<float*>(&results[i].mismatched[0])[k] - reinterpret_cast<float*>(&results[i].mismatched[j])[k];
                        if (lab::Abs(diff) > epsilon) {
                            biggerThanEpsilon = true;
                            break;
                        }
                    }
                }
                if (!biggerThanEpsilon) {
                    results.erase(results.begin() + i);
                    i--;
                }
            }
            
            if (results.size() > 0) {
                printf("\n%s - mismatches found:%lld\n", name.c_str(), results.size());
            }
            
            for (uint32_t i = 0; i < results.size(); i++) {
                for (uint32_t j = 1; j < results[i].mismatched.size(); j++) {
                    constexpr std::size_t resultSize = sizeof(results[i].mismatched[0]) / sizeof(float);
                    for (uint32_t k = 0; k < resultSize; k++) {
                        const float diff = reinterpret_cast<float*>(&results[i].mismatched[0])[k] - reinterpret_cast<float*>(&results[i].mismatched[j])[k];
                        printf("\tdiff[%d][%d] - %.5f\n", i, k, diff);
                    }
                }
            }
            
        }
    };
}
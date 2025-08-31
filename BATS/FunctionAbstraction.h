#pragma once

#include <tuple>
#include <type_traits>
#include <array>
#include <functional>
#include <chrono>

#include "Support/FunctionTraits.h"
#include "Support/Supporting.h"
#include "Support/Results.h"

#ifdef _MSC_VER
#define STRINGIFY(x) #x
#define TOSTRING(x) #x

#pragma message(__FILE__ "(" TOSTRING(__LINE__) "): Benchmarking a void function (not Noop)")
#endif

namespace BATS {

    template<typename Result, auto... Funcs>
    struct FunctionAbstraction {
        static constexpr auto funcs = std::tuple{ Funcs... };

        template<std::size_t I, typename Class_Object, typename... Args>
        static decltype(auto) Call_ClassObject(Class_Object&& classObj, Args&&... args) {
            constexpr auto F = std::get<I>(funcs);

            using ObjType = Function_Traits<std::decay_t<decltype(F)>>::Class_Type;
            ObjType& realObj = static_cast<ObjType&>(classObj);
            using FuncResult = Function_Traits<std::decay_t<decltype(F)>>::Return_Type;

            if constexpr (std::is_void_v<FuncResult>) {
                (realObj.*F)(std::forward<Args>(args)...);
                return;
            }
            else {
                return (realObj.*F)(std::forward<Args>(args)...);
            }
        }

        template <std::size_t I, typename... Args>
        static decltype(auto) Call(Args&&... args) {
            constexpr auto F = std::get<I>(funcs);
            if constexpr (F == nullptr) {
                return;
            }
            else if constexpr (Function_Traits<std::decay_t<decltype(F)>>::is_member_function) {
                static_assert(sizeof...(Args) > 0, "Member function requires object as first argument");
				return Call_ClassObject<I>(std::forward<Args>(args)...);
            }
            else {
                using FuncResult = std::invoke_result_t<decltype(F), Args...>;
                using FirstParamType = first_arg_t<decltype(F)>;
                if constexpr (std::is_same_v<FuncResult, Result> || std::is_convertible_v<FuncResult, Result>) {
                    if constexpr (std::is_void_v<Result>) {
                        std::get<I>(funcs)(std::forward<Args>(args)...);
                        return;
                    }
                    else {
                        return std::get<I>(funcs)(std::forward<Args>(args)...);
                    }
                }
                else if constexpr(std::is_same_v<Result, std::decay_t<FirstParamType>>) {
                    using RetType = std::decay_t<FirstParamType>;
                    static_assert(std::is_same_v<RetType, Result> || std::is_convertible_v<RetType, Result>, "First param should match the return type");
                    RetType ret{};
                    std::get<I>(funcs)(ret, std::forward<Args>(args)...);
                    return ret;
                }
                else if constexpr (std::is_same_v<Result*, std::decay_t<FirstParamType>>) {
                    using RetType = std::remove_pointer_t<std::decay_t<FirstParamType>>;
                    RetType ret{};
                    std::get<I>(funcs)(&ret, std::forward<Args>(args)...);
                    return ret;
                }
                else {
                    static_assert(std::is_lvalue_reference_v<FirstParamType> || std::is_pointer_v<FirstParamType>, "if the first param isn't the out param, the function needs special handling");
                }
            }
        }

        template<std::size_t... Is, typename... Args>
        static std::array<std::optional<Result>, sizeof...(Funcs)>
            Call_All_Impl(std::index_sequence<Is...>, Args&&... args) {
            return { Call<Is>(std::forward<Args>(args)...)... };
        }

        template<typename... Args>
        static std::array<std::optional<Result>, sizeof...(Funcs)> Call_All(Args&&... args) {
            return Call_All_Impl(std::make_index_sequence<sizeof...(Funcs)>{}, std::forward<Args>(args)...);
        }
        template<typename... Args>
        static void Call_All_Print(Args&&... args) {
            static_assert(Printable<Result>, "attempting to print with a non-printable type");
            auto results = Call_All_Impl(std::make_index_sequence<sizeof...(Funcs)>{}, std::forward<Args>(args)...);
            for (auto& ret : results) {
                if (ret.has_value()) {
                    ret->Print();
                }
            }
        }
        template<typename PrintFunc, typename... Args>
        static void Call_All_PrintFunc(PrintFunc&& printFunc, Args&&... args) {
            auto results = Call_All_Impl(std::make_index_sequence<sizeof...(Funcs)>{}, std::forward<Args>(args)...);
            for (auto& ret : results) {
                if (ret.has_value()) {
                    printFunc(*ret);
                }
            }
        }

        template<std::size_t I, typename... Args>
        static void BenchmarkOne_Time(BenchmarkResults<sizeof...(Funcs)>& results, Args&&... args) {
            const auto start = std::chrono::high_resolution_clock::now();
            auto temp = Call<I>(std::forward<Args>(args)...);
            const auto end = std::chrono::high_resolution_clock::now();
            DoNotOptimize(temp);

            const auto duration = end - start;
            results.total[I] += duration;
            MAX_EQUALS(results.max[I], duration);
			MIN_EQUALS(results.min[I], duration);
        }

        template<std::size_t... Is, typename... Args>
        static void Benchmark_OneEach_Helper_Time(BenchmarkResults<sizeof...(Funcs)>& results, std::index_sequence<Is...>, Args&&... args) {
            (BenchmarkOne_Time<Is>(results, std::forward<Args>(args)...), ...);
        }

        template<typename... Args>
        static BenchmarkResults<sizeof...(Funcs)> Benchmark_OneEach_Time(uint64_t iterationCount, Args&&... args) {
            BenchmarkResults<sizeof...(Funcs)> results(iterationCount);

            for (uint64_t i = 0; i < iterationCount; ++i) {
                Benchmark_OneEach_Helper_Time(results, std::make_index_sequence<sizeof...(Funcs)>{}, std::forward<Args>(args)...);
            }

            return results;
        }

        template<std::size_t I, typename... Args>
        static void Benchmark_Batch_Time(const uint64_t batchSize, BenchmarkResults<sizeof...(Funcs)>& results, Args&&... args) {
            const auto start = std::chrono::high_resolution_clock::now();

            for (uint64_t i = 0; i < batchSize; ++i) {
                if constexpr (std::is_same_v<Result, void>) {
                    if constexpr (std::get<I>(funcs) != &Noop) {
                        //compiler warning
#ifdef _MSC_VER
                        __pragma(message(__FILE__ "(" TOSTRING(__LINE__)"): warning C4999: Benchmarking a void function (not Noop). Context:" __FUNCSIG__))
#endif
                    }
                    Call<I>(std::forward<Args>(args)...);
                }
                else {
                    auto temp = Call<I>(std::forward<Args>(args)...);
                    DoNotOptimize(temp);
                }
            }
            const auto end = std::chrono::high_resolution_clock::now();

            const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>((end - start) / batchSize);
            results.total[I] += duration;
            MAX_EQUALS(results.max[I], duration);
            MIN_EQUALS(results.min[I], duration);
        }
        template<std::size_t... Is, typename... Args>
        static void Benchmark_Batch_Helper_Time(const uint64_t batchSize, BenchmarkResults<sizeof...(Funcs)>& results, std::index_sequence<Is...>, Args&&... args) {
            (Benchmark_Batch_Time<Is>(batchSize, results, std::forward<Args>(args)...), ...);
        }

        template<typename... Args>
        static BenchmarkResults<sizeof...(Funcs)> Benchmark_Batch_Time(uint64_t batchCount, uint64_t batchSize, Args&&... args) {
            BenchmarkResults<sizeof...(Funcs)> results(batchCount);

            for (uint64_t i = 0; i < batchCount; ++i) {
                Benchmark_Batch_Helper_Time(batchSize, results, std::make_index_sequence<sizeof...(Funcs)>{}, std::forward<Args>(args)...);
            }

            return results;
        }





        template<std::size_t I, typename Tuple, std::size_t... Js>
        static Result Benchmark_Each_Helper_Helper_Accuracy(std::index_sequence<Js...>, Tuple&& args) {
            if constexpr (std::is_void_v<Result>) {
                Call<I>(std::get<Js>(std::forward<Tuple>(args))...);
                return;
            }
            else {
                return Call<I>(std::get<Js>(std::forward<Tuple>(args))...);
            }
        }

        template<std::size_t... Is, typename Tuple>
        static std::array<Result, sizeof...(Is)>
            Benchmark_Each_Helper_Accuracy(std::index_sequence<Is...>, Tuple&& args) {
            return { Benchmark_Each_Helper_Helper_Accuracy<Is>(
                         std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{},
                         std::forward<Tuple>(args))... };
        }

        
        template<typename U, typename... Args>
        static std::vector<AccuracyResult<sizeof...(Funcs), Result, std::tuple<Args...>>> Benchmark_Each_Accuracy(uint64_t iterationCount, U random_lowerBound, U random_upperBound) {
            std::vector<AccuracyResult<sizeof...(Funcs), Result, std::tuple<Args...>>> results{};

            for (uint64_t i = 0; i < iterationCount; ++i) {
                auto args = GenerateRandomTuple<U, Args...>(random_lowerBound, random_upperBound);
                std::array<Result, sizeof...(Funcs)> ret = Benchmark_Each_Helper_Accuracy(std::make_index_sequence<sizeof...(Funcs)>{}, args);
                results.emplace_back(ret, args);
            }

            return results;
        }


    };


	using NoopTest = FunctionAbstraction<void, Noop>;
}
#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <optional>
#include <array>
#include <functional>

#include <chrono>

#ifdef _MSC_VER
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#pragma message(__FILE__ "(" TOSTRING(__LINE__) "): Benchmarking a void function (not Noop)")
#endif

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

    template <typename T>
    struct function_traits;

    template <typename Result, typename... Args>
    struct function_traits<Result(*)(Args...)> {
        using result_type = Result;
        using args_tuple = std::tuple<Args...>;
        static constexpr std::size_t arity = sizeof...(Args);
    };
    template <typename Func>
    using first_arg_t = std::tuple_element_t<0, typename function_traits<std::decay_t<Func>>::args_tuple>;


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

    template<typename Result, auto... Funcs>
    struct FunctionAbstraction {
        static constexpr auto funcs = std::tuple{ Funcs... };

        template <std::size_t I, typename... Args>
        static decltype(auto) Call(Args&&... args) {
            constexpr auto F = std::get<I>(funcs);
            if constexpr (F == nullptr) {
                return;
            }
            else {
                using FuncResult = std::invoke_result_t<decltype(F), Args...>;
                if constexpr (std::is_same_v<FuncResult, Result> || std::is_convertible_v<FuncResult, Result>) {
                    if constexpr (std::is_void_v<Result>) {
                        std::get<I>(funcs)(std::forward<Args>(args)...);
                        return;
                    }
                    else {
                        return std::get<I>(funcs)(std::forward<Args>(args)...);
                    }
                }
                else {
                    using FirstParamType = first_arg_t<decltype(F)>;
                    static_assert(std::is_lvalue_reference_v<FirstParamType>, "if the first param isn't the out param, the function needs special handling");
                    using RetType = std::remove_reference_t<FirstParamType>;
                    static_assert(std::is_same_v<RetType, Result> || std::is_convertible_v<RetType, Result>, "First param should match the return type");
                    RetType ret{};
                    std::get<I>(funcs)(ret, std::forward<Args>(args)...);
                    return ret;
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
    };


	using NoopTest = FunctionAbstraction<void, Noop>;
}
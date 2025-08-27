#include "PrintAbstraction.h"
#include "../BATS/FunctionAbstraction.h"
#include "LAB/Vector.h"
#include "LAB/Matrix.h"
#include "LAB/Quaternion.h"

#include "TypeAbstract.h"
#include "d3dx9math.h"

#include <cstdint>
#include <cstdio>
#include <functional>


//might want to work this in a linux distro
//https://www.mycpu.org/writing-bpf-c++/


template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>; // require that result is same type T
};

template<Addable T>
T AddT(T a, T b) {
    return a + b;
}

using VecAbstract2 = TypeAbstraction<lab::Vector<float, 2>, D3DXVECTOR2>;
using VecAbstract3 = TypeAbstraction<lab::Vector<float, 3>, D3DXVECTOR3>;
using VecAbstract4 = TypeAbstraction<lab::Vector<float, 4>, D3DXVECTOR4>;
using QuatAbst = TypeAbstraction<lab::Quaternion<float>, D3DXQUATERNION>;
using MatAbst = TypeAbstraction<lab::Matrix<float, 4, 4>, D3DXMATRIX>;

using Vec2AddAbst = BATS::FunctionAbstraction<VecAbstract2, AddT<lab::Vector<float, 2>>, AddT<D3DXVECTOR2>>;
using Vec3AddAbst = BATS::FunctionAbstraction<VecAbstract3, AddT<lab::Vector<float, 3>>, AddT<D3DXVECTOR3>>;
using Vec4AddAbst = BATS::FunctionAbstraction<VecAbstract4, AddT<lab::Vector<float, 4>>, AddT<D3DXVECTOR4>>;

using MatPtr = lab::Matrix<float, 4, 4>(lab::Matrix<float, 4, 4>::*)(const lab::Matrix<float, 4, 4>&) const;

using Mat4MultiAbst = BATS::FunctionAbstraction<
    MatAbst, 
    static_cast<lab::Matrix<float, 4, 4>(lab::Matrix<float, 4, 4>::*)(const lab::Matrix<float, 4, 4>&) const>(&lab::Matrix<float, 4, 4>::operator*), 
    static_cast<D3DXMATRIX(D3DXMATRIX::*)(const D3DXMATRIX&) const>(
        &D3DXMATRIX::operator*)
>;

void TestFunc() {
    int testing = 0;
}

int main(){
    VecAbstract2 vector2A(lab::Vector<float, 2>{4.f, 1.f});
    VecAbstract2 vector2B(lab::Vector<float, 2>{3.f, 3.f});

    
    {
        VecAbstract2 ret = Vec2AddAbst::Call<0>(vector2A, vector2B);
        Print(ret);

        auto arrayRet = Vec2AddAbst::Call_All(vector2A, vector2B);
        printf("\narray ret\n");
        for (auto& re : arrayRet) {
            Print(*re);
        }
    }
    printf("\nall ret1\n");
    Vec2AddAbst::Call_All_PrintFunc(&Print<VecAbstract2>, vector2A, vector2B);
    printf("\nall ret2\n");

    VecAbstract3 vec3A(lab::Vector<float, 3>{ 0.f, 1.f, 2.f });
    VecAbstract3 vec3B(lab::Vector<float, 3>{ 4.f, 5.f, 6.f });
    Vec3AddAbst::Call_All_PrintFunc(&Print<VecAbstract3>, vec3A, vec3B);
	VecAbstract4 vec4A(lab::Vector<float, 4>{ 0.f, 1.f, 2.f, 3.f });
	VecAbstract4 vec4B(lab::Vector<float, 4>{ 5.f, 6.f, 7.f, 8.f });
    Vec4AddAbst::Call_All_PrintFunc(&Print<VecAbstract4>, vec4A, vec4B);

    QuatAbst quat{lab::Quaternion<float>(0.f, 1.f, 2.f, 3.f)};
    {
        auto results = Vec4AddAbst::Benchmark_OneEach_Time(1000, vec4A, vec4B);
		printf("\none each results:\n");
        for (auto& ret : results.total) {
			printf("Total: %lld ns\n", ret.count());
            printf("Avg: %lld ns\n", ret.count() / results.iteration_count);
        }
    }
    {
        auto results = Vec4AddAbst::Benchmark_Batch_Time(100, 10000, vec4A, vec4B);
		printf("\nbatch results:\n");
        for (auto& ret : results.total) {
            printf("Total: %lld ns\n", ret.count());
            printf("Avg: %lld ns\n", ret.count() / results.iteration_count);
        }
    }
    {
        auto results = BATS::NoopTest::Benchmark_Batch_Time(100, 1000);
        printf("\nnoop iters results:\n");
        for (auto& ret : results.total) {
            printf("Total: %lld ns\n", ret.count());
            printf("Avg: %lld ns\n", ret.count() / results.iteration_count);
        }
    }

    {
        auto results = BATS::FunctionAbstraction<void, TestFunc>::Benchmark_Batch_Time(10, 10);
    }
    {
        auto mismatchResults = Vec4AddAbst::Benchmark_Each_Accuracy<float, VecAbstract4, VecAbstract4>(5, -100.f, 100.f);
        printf("\nmismatch size : %lld", mismatchResults.size());

        //i was debugging here step by step here, leaving it in casei need to debug again later
        //auto temp1 = BATS::random_value<VecAbstract4>(-100.f, 100.f);
        //auto temp2 = BATS::random_value<VecAbstract4>(-100.f, 100.f);
		//auto args = BATS::Function_Traits<Vec4AddAbst::first_func_t>::random_args(-100.f, 100.f);
		//auto random_tuple = BATS::GenerateRandomTuple<float, VecAbstract4, VecAbstract4>(-100.f, 100.f);
    }
    {
		auto mismatchResults = Mat4MultiAbst::Benchmark_Each_Accuracy<float, MatAbst, MatAbst>(5, -100.f, 100.f);
        printf("\nmismatch size : %lld", mismatchResults.size());
    }
    return EXIT_SUCCESS;
}
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

template <typename T>
constexpr auto AddrIfNotFundamental(T& value) {
    if constexpr (std::is_fundamental_v<T> || std::is_enum_v<T>) {
        return value;  // return by value
    }
    else {
        return &value; // return pointer for classes/structs
    }
}

template<auto Func>
struct DXWrapper;

// Specialization for functions like R* f(R*, Args...)
template<typename R, typename... Args, R* (WINAPI* Func)(R*, Args...)>
struct DXWrapper<Func> {
    using Return = R;

    static Return Call(std::remove_pointer_t<Args>... args) {
        R out{};
        Func(&out, AddrIfNotFundamental(args)...);
        return out;
    }
};

using VecAbstract2 = TypeAbstraction<lab::Vector<float, 2>, D3DXVECTOR2>;
using VecAbstract3 = TypeAbstraction<lab::Vector<float, 3>, D3DXVECTOR3>;
using VecAbstract4 = TypeAbstraction<lab::Vector<float, 4>, D3DXVECTOR4>;

using QuatAbst = TypeAbstraction<lab::Quaternion<float>, D3DXQUATERNION>;
using MatAbst = TypeAbstraction<lab::Matrix<float, 4, 4>, D3DXMATRIX>;

using Vec2AddAbst = BATS::FunctionAbstraction<VecAbstract2, AddT<lab::Vector<float, 2>>, AddT<D3DXVECTOR2>>;
using Vec3AddAbst = BATS::FunctionAbstraction<VecAbstract3, AddT<lab::Vector<float, 3>>, AddT<D3DXVECTOR3>>;
using Vec4AddAbst = BATS::FunctionAbstraction<VecAbstract4, AddT<lab::Vector<float, 4>>, AddT<D3DXVECTOR4>>;


D3DXQUATERNION DxAngleAxis(float angle, D3DXVECTOR3 vec) {
    D3DXQUATERNION ret;
    D3DXQuaternionRotationAxis(&ret, &vec, angle);
    return ret;
}




D3DXMATRIX FlippedInputRotationAxis(float angle, D3DXVECTOR3 axis) {
    D3DXMATRIX ret;
    D3DXMatrixRotationAxis(&ret, &axis, angle);
    return ret;
}


lab::vec3 LAB_VEC3_MAT(lab::vec3 const& in_vec, lab::mat4 const& mat) {
    const lab::vec4 tempVec(in_vec, 1.f);
    const lab::vec4 out = tempVec * mat;
    return lab::vec3(out) / out.w;
}
D3DXVECTOR3 DX_VEC3_MAT(D3DXVECTOR3& in_vec, D3DXMATRIX& mat) {
    D3DXVECTOR3 out;

    FLOAT x = in_vec.x * mat._11 + in_vec.y * mat._21 + in_vec.z * mat._31 + mat._41;
    FLOAT y = in_vec.x * mat._12 + in_vec.y * mat._22 + in_vec.z * mat._32 + mat._42;
    FLOAT z = in_vec.x * mat._13 + in_vec.y * mat._23 + in_vec.z * mat._33 + mat._43;
    FLOAT w = in_vec.x * mat._14 + in_vec.y * mat._24 + in_vec.z * mat._34 + mat._44;

    out.x = x / w;
    out.y = y / w;
    out.z = z / w;

    return out;
}



void TestFunc() {
    int testing = 0;
}

template<typename T>
void PrintResults(std::string const& name, std::vector<T>& results, const float epsilon) {
    T::Print(name, results, epsilon);
}

D3DXMATRIX DXInverse(D3DXMATRIX input) {
    D3DXMATRIX ret;
    float det;
    D3DXMatrixInverse(&ret, &det, &input);
    return ret;
}

int main(){
    /*
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
    */
    const float epsilon = 0.0001f;// 1f;
    {
        auto mismatchResults = Vec4AddAbst::Benchmark_Each_Accuracy<float, VecAbstract4, VecAbstract4>(50, -100.f, 100.f);
        PrintResults("vec4+", mismatchResults, epsilon);

        //i was debugging here step by step here, leaving it in casei need to debug again later
        //auto temp1 = BATS::random_value<VecAbstract4>(-100.f, 100.f);
        //auto temp2 = BATS::random_value<VecAbstract4>(-100.f, 100.f);
		//auto args = BATS::Function_Traits<Vec4AddAbst::first_func_t>::random_args(-100.f, 100.f);
		//auto random_tuple = BATS::GenerateRandomTuple<float, VecAbstract4, VecAbstract4>(-100.f, 100.f);
    }
    {
        using Mat4MultiAbst = BATS::FunctionAbstraction<
            MatAbst,
            static_cast<lab::Matrix<float, 4, 4>(lab::Matrix<float, 4, 4>::*)(const lab::Matrix<float, 4, 4>&) const>(&lab::Matrix<float, 4, 4>::operator*),
            static_cast<D3DXMATRIX(D3DXMATRIX::*)(const D3DXMATRIX&) const>(
                &D3DXMATRIX::operator*)
        >;
        auto mismatchResults = Mat4MultiAbst::Benchmark_Each_Accuracy<float, MatAbst, MatAbst>(50, -100.f, 100.f);
        PrintResults("m4*m4", mismatchResults, epsilon);
    }
    {
        using CrossAbst = BATS::FunctionAbstraction<
            VecAbstract3,
            lab::Cross<float, 3>,
            &DXWrapper<D3DXVec3Cross>::Call
        >;
        auto mismatchResults = CrossAbst::Benchmark_Each_Accuracy<float, VecAbstract3, VecAbstract3>(50, -100.f, 100.f);
        PrintResults("cross", mismatchResults, epsilon);
    }
    {
        using DotAbst = BATS::FunctionAbstraction<
            float,
            lab::Dot<float, 3>,
            D3DXVec3Dot
        >;
        auto mismatchResults = DotAbst::Benchmark_Each_Accuracy<float, VecAbstract3, VecAbstract3>(50, -100.f, 100.f);
        PrintResults("dot", mismatchResults, epsilon);
    }
	{
        using AngleAxisAbst = BATS::FunctionAbstraction<
            QuatAbst,
            lab::Quat::AngleAxis,
            DxAngleAxis
        >;
		auto mismatchResults = AngleAxisAbst::Benchmark_Each_Accuracy<float, float, VecAbstract3>(50, -100.f, 100.f);
        PrintResults("angle axis", mismatchResults, epsilon);
	}
    {
        using MatFromQuatAbst = BATS::FunctionAbstraction <
            MatAbst,
            static_cast<lab::Matrix<float, 4, 4>(*)(lab::Quat const&)>(&lab::Quat::ToMat4),
            DXWrapper<D3DXMatrixRotationQuaternion>::Call
        >;
        auto mismatchResults = MatFromQuatAbst::Benchmark_Each_Accuracy<float, QuatAbst>(50, -100.f, 100.f);
        PrintResults("mat from quat", mismatchResults, epsilon);
    }
    {
        using QuatFromMatAbst = BATS::FunctionAbstraction<
            QuatAbst,
            lab::Quat::FromMatrix,
            DXWrapper<D3DXQuaternionRotationMatrix>::Call
        >;
        auto mismatchResults = QuatFromMatAbst::Benchmark_Each_Accuracy<float, MatAbst>(50, -100.f, 100.f);
        PrintResults("quat from mat", mismatchResults, epsilon);
    } 
    {
        using Vec3Mat4Abst = BATS::FunctionAbstraction<
            VecAbstract3,
            LAB_VEC3_MAT,
            DX_VEC3_MAT
        >;
        auto mismatchResults = Vec3Mat4Abst::Benchmark_Each_Accuracy<float, VecAbstract3, MatAbst>(50, -100.f, 100.f);
        PrintResults("vec3 * mat4", mismatchResults, epsilon);
    }
    {
        using RotXAbst = BATS::FunctionAbstraction<
            MatAbst,
            lab::RotateAroundX<float>,
            DXWrapper<D3DXMatrixRotationX>::Call
        >;
		auto mismatchResults = RotXAbst::Benchmark_Each_Accuracy<float, float>(50, -100.f, 100.f);
		PrintResults("rot x", mismatchResults, epsilon);
	}
    {
        using RotYAbst = BATS::FunctionAbstraction<
            MatAbst,
            lab::RotateAroundY<float>,
            DXWrapper<D3DXMatrixRotationY>::Call
        >;
        auto mismatchResults = RotYAbst::Benchmark_Each_Accuracy<float, float>(50, -100.f, 100.f);
        PrintResults("rot y", mismatchResults, epsilon);
    }
    {
        using RotZAbst = BATS::FunctionAbstraction<
            MatAbst,
            lab::RotateAroundZ<float>,
            DXWrapper<D3DXMatrixRotationZ>::Call
        >;
        auto mismatchResults = RotZAbst::Benchmark_Each_Accuracy<float, float>(50, -100.f, 100.f);
        PrintResults("rot z", mismatchResults, epsilon);
    }
    {
        using RotAxisAbst = BATS::FunctionAbstraction<
            MatAbst,
            lab::RotateAroundAxis<float>,
            FlippedInputRotationAxis
        >;
        auto mismatchResults = RotAxisAbst::Benchmark_Each_Accuracy<float, float, VecAbstract3>(50, -100.f, 100.f);
        PrintResults("rot axis", mismatchResults, epsilon);
    }
    {
        auto mismatchResults = BATS::FunctionAbstraction<
            QuatAbst,
            lab::Quaternion<float>::Slerp,
            DXWrapper<D3DXQuaternionSlerp>::Call
        >::Benchmark_Each_Accuracy<float, QuatAbst, QuatAbst, float>(50, 0.f, 1.f);
        PrintResults("quat slerp", mismatchResults, epsilon);
    }
    {
        auto mismatchResults = BATS::FunctionAbstraction<
            MatAbst,
            &lab::Matrix<float, 4, 4>::GetInverse,
            DXInverse
		>::Benchmark_Each_Accuracy<float, MatAbst>(50, -100.f, 100.f);
		PrintResults("mat inverse", mismatchResults, epsilon);
    }
    {
        auto mismatchResults = BATS::FunctionAbstraction<
            float,
            &lab::Matrix<float, 4, 4>::GetDeterminant,
            D3DXMatrixDeterminant
		>::Benchmark_Each_Accuracy<float, MatAbst>(50, -100.f, 100.f);
        PrintResults("mat determ", mismatchResults, epsilon);
    }
    {

    }
	

    return EXIT_SUCCESS;
}
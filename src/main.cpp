#include "PrintAbstraction.h"
#include "FunctionAbstraction.h"
#include "LAB/Vector.h"
#include "LAB/Matrix.h"
#include "LAB/Quaternion.h"

#include "TypeAbstract.h"
#include "d3dx9math.h"

#include <cstdint>
#include <cstdio>


lab::Vector<float, 2> AddLab(lab::Vector<float, 2> a, lab::Vector<float, 2> b){
    return lab::Vector<float, 2>{
        a.x + b.x,
        a.y + b.y
    };
}
D3DXVECTOR2 AddDX(D3DXVECTOR2 a, D3DXVECTOR2 b){
    return D3DXVECTOR2{
        a.x + b.x,
        a.y + b.y
    };
}

using VecAbstract2 = TypeAbstraction<lab::Vector<float, 2>, D3DXVECTOR2>;
using QuatAbstraction = TypeAbstraction<lab::Quaternion<float>, D3DXQUATERNION>;


int main(){
    VecAbstract2 vectorA(lab::Vector<float, 2>{4.f, 1.f});
    VecAbstract2 vectorB(lab::Vector<float, 2>{3.f, 3.f});

    VecAbstract2 ret = FunctionAbstraction<VecAbstract2, AddLab, AddDX>::Call<0>(vectorA, vectorB);

    Print(ret);

    auto arrayRet = FunctionAbstraction<VecAbstract2, AddLab, AddDX>::Call_All(vectorA, vectorB);
    printf("\narray ret\n");
    for(auto& re : arrayRet){
        Print(*re);
    }

    printf("\nall ret1\n");
    FunctionAbstraction<VecAbstract2, AddLab, AddDX>::Call_All_PrintFunc(&Print<VecAbstract2>, vectorA, vectorB);
    printf("\nall ret2\n");

    QuatAbstraction quat{lab::Quaternion<float>(0.f, 1.f, 2.f, 3.f)};



    return EXIT_SUCCESS;
}
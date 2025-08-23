#include "VectorAbstraction.h"
#include "FunctionAbstraction.h"

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


int main(){

    VectorAbstraction<2> vectorA{0.f, 1.f};
    VectorAbstraction<2> vectorB{3.f, 1.f};

    constexpr FunctionAbstraction<VectorAbstraction<2>, AddLab, AddDX> funcAbs;

    VectorAbstraction<2> ret = funcAbs.Call<0>(vectorA, vectorB);

    printf("ret.x, ret.y - %.2f:%.2f\n", ret.labVec.x, ret.labVec.y);

    auto arrayRet = funcAbs.Call_All(vectorA, vectorB);
    for(auto& re : arrayRet){
        re->Print();
    }

    funcAbs.Call_All_Print(vectorA, vectorB);



    return EXIT_SUCCESS;
}
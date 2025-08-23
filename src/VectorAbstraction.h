#pragma once

#include "LAB/Vector.h"
#include "d3dx9math.h"

#include <cstdint>
#include <cstdio>

template<uint8_t Dimensions>
requires(Dimensions > 1 && Dimensions <= 4)
struct VectorAbstraction{

    using DXVector = std::conditional_t<
        Dimensions == 2, D3DXVECTOR2,
        std::conditional_t<
            Dimensions == 3, D3DXVECTOR3,
            std::conditional_t<
                Dimensions == 4, D3DXVECTOR4,
                void // fallback if out of range
            >
        >
    >;
    template<typename... Ts>
    VectorAbstraction(Ts... vals) {
        static_assert(sizeof...(vals) == Dimensions, "Number of values must match the dimension");

        dxVec = DXVector{static_cast<float>(vals)...};
        labVec = lab::Vector<float, Dimensions>{static_cast<float>(vals)...};
    }
    VectorAbstraction(lab::Vector<float, Dimensions> const& labVec) : labVec{labVec} {
        memcpy(&dxVec, &labVec, sizeof(labVec));
    }
    VectorAbstraction(DXVector const& dxVec) : dxVec{dxVec}{
        memcpy(&labVec, &dxVec, sizeof(dxVec));
    }


    void Print(){
        printf("lab - ");
        for(uint8_t i = 0; i < Dimensions; i++){
            printf("(%.2f)", labVec[i]);
        }

        if constexpr(Dimensions == 2){
            printf("\ndx - (%.2f)(%.2f)\n", dxVec.x, dxVec.y);
        }
        if constexpr(Dimensions == 3){
            printf("\ndx - (%.2f)(%.2f)(%.2f)\n", dxVec.x, dxVec.y, dxVec.z);
        }
        else if constexpr(Dimensions == 4){
            printf("\ndx - (%.2f)(%.2f)(%.2f)(%.2f)\n", dxVec.x, dxVec.y, dxVec.z, dxVec.w);
        }
    }

    DXVector dxVec;
    lab::Vector<float, Dimensions> labVec;

    operator DXVector&(){ return dxVec;}
    operator const DXVector &() const {return dxVec; }
    operator lab::Vector<float, Dimensions>&(){return labVec;}
    operator const lab::Vector<float, Dimensions>&() const {return labVec;}
};
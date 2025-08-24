#pragma once

#include "TypeAbstract.h"

#include "LAB/Vector.h"
#include "LAB/Matrix.h"
#include "LAB/Quaternion.h"

#include <cstdint>
#include <cstdio>

    template<uint8_t Dimensions>
    void VectorPrint(lab::Vector<float, Dimensions> const& vec){
        printf("vector - ");
        for(uint8_t i = 0; i < Dimensions; i++){
            printf("(%.2f)", vec[i]);
        }
        printf("\n");
    }

    //4d only rn
    void MatrixPrint(lab::Matrix<float, 4, 4> const& mat) {
        printf("matrix - \n");
        for(uint8_t x = 0; x < 4; x++){
            printf("\t");
            for(uint8_t y = 0; y < 4; y++){
                printf("(%.2f)", mat.At(x, y));
            }
        }
    }

    template<typename T>
    void Print(T const& value){
        if constexpr(std::is_same_v<lab::Quaternion<float>, T> || std::is_convertible_v<lab::Quaternion<float>, T>) {
            lab::Quaternion<float> const& quat = value.value;
            printf("quat - (%.2f)(%.2f)(%.2f)(%.2f)\n", quat.x, quat.y, quat.z, quat.w);
        }
        else if constexpr (requires { value.value[0]; }) {
            VectorPrint(value.value);
        }
        else if constexpr (requires { value.value.At(0,0); }) {
            MatrixPrint(value.value);
        }
        else{
            static_assert(false);
        }
    }
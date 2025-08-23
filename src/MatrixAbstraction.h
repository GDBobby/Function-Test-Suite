#pragma once

#include "LAB/Matrix.h"
#include "d3dx9math.h"

template<uint8_t Columns, uint8_t Rows>
requires(Columns > 1 && Columns <= 4 && Rows > 1 && Rows <= 4
//temporary requiremnt here, i dont think d3dx has 3x3 matrix
&& Columns == 4 && Rows == 4
)
struct MatrixAbstraction{

    D3DXMATRIX dxMat;
    lab::Matrix<float, 4, 4> labMat;


    operator D3DXMATRIX&(){ return dxMat;}
    operator const DD3DXMATRIX &() const {return dxMat; }
    operator lab::Matrix<float, 4, 4>&(){return labMat;}
    operator const lab::Matrix<float, 4, 4>&() const {return labMat;}
};
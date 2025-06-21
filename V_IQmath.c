/*!
    Copyright 2017 	АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      V_IQmath.c
 \brief     Библиотека функций целочисленной математики
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup V_IQmath 
 @{*/

#include "V_Include/V_IQmath.h"
#include "stdlib.h"
#include "math.h"

#pragma CODE_SECTION(_IQ24sinPU, "ramfuncs");

//! Деление двух чисел в формате 22.10

//! \memberof V_IQmath
int32 _IQ10div(int32 a, int32 b) {
	if (b == 0)
		return 0;
	return _IQ10((float )(a) / b); //32 такта
}

//! Деление двух чисел в формате 8.24
//! \memberof V_IQmath
int32 _IQ24div(int32 a, int32 b) {
	if (b == 0)
		return 0;
	return _IQ24((float )a / b); //32 такта
}


//! Квадратный корень в формате 8.24. Работает через аппаратный float, занимает 43 такта.
//! В CodeMaster нет функции sqrtf, поэтому для него вызывается sqrt. Для GCC вызывать
//! sqrt нельзя, т.к. тогда он попытается работать с double, а не float.
int32 _IQ24sqrt(int32 number) {
	return _IQ(sqrtf((float)_IQtoF(number)));
}



static const int16 sinTableQ15[259] = {
    0xfcdc, 0x0, 0x324, 0x648, 0x96b, 0xc8c, 0xfab, 0x12c8,
    0x15e2, 0x18f9, 0x1c0c, 0x1f1a, 0x2224, 0x2528, 0x2827, 0x2b1f,
    0x2e11, 0x30fc, 0x33df, 0x36ba, 0x398d, 0x3c57, 0x3f17, 0x41ce,
    0x447b, 0x471d, 0x49b4, 0x4c40, 0x4ec0, 0x5134, 0x539b, 0x55f6,
    0x5843, 0x5a82, 0x5cb4, 0x5ed7, 0x60ec, 0x62f2, 0x64e9, 0x66d0,
    0x68a7, 0x6a6e, 0x6c24, 0x6dca, 0x6f5f, 0x70e3, 0x7255, 0x73b6,
    0x7505, 0x7642, 0x776c, 0x7885, 0x798a, 0x7a7d, 0x7b5d, 0x7c2a,
    0x7ce4, 0x7d8a, 0x7e1e, 0x7e9d, 0x7f0a, 0x7f62, 0x7fa7, 0x7fd9,
    0x7ff6, 0x7fff, 0x7ff6, 0x7fd9, 0x7fa7, 0x7f62, 0x7f0a, 0x7e9d,
    0x7e1e, 0x7d8a, 0x7ce4, 0x7c2a, 0x7b5d, 0x7a7d, 0x798a, 0x7885,
    0x776c, 0x7642, 0x7505, 0x73b6, 0x7255, 0x70e3, 0x6f5f, 0x6dca,
    0x6c24, 0x6a6e, 0x68a7, 0x66d0, 0x64e9, 0x62f2, 0x60ec, 0x5ed7,
    0x5cb4, 0x5a82, 0x5843, 0x55f6, 0x539b, 0x5134, 0x4ec0, 0x4c40,
    0x49b4, 0x471d, 0x447b, 0x41ce, 0x3f17, 0x3c57, 0x398d, 0x36ba,
    0x33df, 0x30fc, 0x2e11, 0x2b1f, 0x2827, 0x2528, 0x2224, 0x1f1a,
    0x1c0c, 0x18f9, 0x15e2, 0x12c8, 0xfab, 0xc8c, 0x96b, 0x648,
    0x324, 0x0, 0xfcdc, 0xf9b8, 0xf695, 0xf374, 0xf055, 0xed38,
    0xea1e, 0xe707, 0xe3f4, 0xe0e6, 0xdddc, 0xdad8, 0xd7d9, 0xd4e1,
    0xd1ef, 0xcf04, 0xcc21, 0xc946, 0xc673, 0xc3a9, 0xc0e9, 0xbe32,
    0xbb85, 0xb8e3, 0xb64c, 0xb3c0, 0xb140, 0xaecc, 0xac65, 0xaa0a,
    0xa7bd, 0xa57e, 0xa34c, 0xa129, 0x9f14, 0x9d0e, 0x9b17, 0x9930,
    0x9759, 0x9592, 0x93dc, 0x9236, 0x90a1, 0x8f1d, 0x8dab, 0x8c4a,
    0x8afb, 0x89be, 0x8894, 0x877b, 0x8676, 0x8583, 0x84a3, 0x83d6,
    0x831c, 0x8276, 0x81e2, 0x8163, 0x80f6, 0x809e, 0x8059, 0x8027,
    0x800a, 0x8000, 0x800a, 0x8027, 0x8059, 0x809e, 0x80f6, 0x8163,
    0x81e2, 0x8276, 0x831c, 0x83d6, 0x84a3, 0x8583, 0x8676, 0x877b,
    0x8894, 0x89be, 0x8afb, 0x8c4a, 0x8dab, 0x8f1d, 0x90a1, 0x9236,
    0x93dc, 0x9592, 0x9759, 0x9930, 0x9b17, 0x9d0e, 0x9f14, 0xa129,
    0xa34c, 0xa57e, 0xa7bd, 0xaa0a, 0xac65, 0xaecc, 0xb140, 0xb3c0,
    0xb64c, 0xb8e3, 0xbb85, 0xbe32, 0xc0e9, 0xc3a9, 0xc673, 0xc946,
    0xcc21, 0xcf04, 0xd1ef, 0xd4e1, 0xd7d9, 0xdad8, 0xdddc, 0xe0e6,
    0xe3f4, 0xe707, 0xea1e, 0xed38, 0xf055, 0xf374, 0xf695, 0xf9b8,
    0xfcdc, 0x0, 0x324
};



//! Функция синуса, табличная, точная, с кубической интерполяцией между точками.

//! Внутри работает с тоностью 16.16, но конвертируется в 8.24
//! 83 такта
//!Взято из библиотеки DSP ARM
//! \memberof V_IQmath
int32 _IQ24sinPU_accurate(
    int32 x) {
    int32 sinVal;                                  /* Temporary variables output */
    int16 *tablePtr;                               /* Pointer to table */
    int16 fract, in, in2;                          /* Temporary variables for input, output */
    int32 wa, wb, wc, wd;                          /* Cubic interpolation coefficients */
    int16 a, b, c, d;                              /* Four nearest output values */
    int16 fractCube, fractSquare;                  /* Temporary values for fractional value */
    int16 oneBy6 = 0x1555;                         /* Fixed point16 value of 1/6 */
    int16 tableSpacing = 0x80;        /* Table spacing */
    int32 index;                                 /* Index variable */

    in = (x>>9)&0x7FFF;


    /* Calculate the nearest index */
    index = (int32_t) in / tableSpacing;

    /* Calculate the nearest value of input */
    in2 = (int16) ((index) * tableSpacing);

    /* Calculation of fractional value */
    fract = (in - in2) << 8;

    /* fractSquare = fract * fract */
    fractSquare = (int16) ((fract * fract) >> 15);

    /* fractCube = fract * fract * fract */
    fractCube = (int16) ((fractSquare * fract) >> 15);

    /* Checking min and max index of table */
    if (index < 0) {
        index = 0;
    } else if (index > 256) {
        index = 256;
    }

    /* Initialise table pointer */
    tablePtr = (int16 *) & sinTableQ15[index];

    /* Cubic interpolation process */
    /* Calculation of wa */
    /* wa = -(oneBy6)*fractCube + (fractSquare >> 1u) - (0x2AAA)*fract; */
    wa = (int32) oneBy6 *fractCube;
    wa += (int32) 0x2AAA *fract;
    wa = -(wa >> 15);
    wa += ((int32) fractSquare >> 1u);

    /* Read first nearest value of output from the sin table */
    a = *tablePtr++;

    /* sinVal = a * wa */
    sinVal = a * wa;

    /* Calculation of wb */
    wb = (((int32) fractCube >> 1u) - (int32) fractSquare) -
         (((int32) fract >> 1u) - 0x7FFF);

    /* Read second nearest value of output from the sin table */
    b = *tablePtr++;

    /*      sinVal += b*wb */
    sinVal += b * wb;


    /* Calculation of wc */
    wc = -(int32) fractCube + fractSquare;
    wc = (wc >> 1u) + fract;

    /* Read third nearest value of output from the sin table */
    c = *tablePtr++;

    /* sinVal += c*wc */
    sinVal += c * wc;

    /* Calculation of wd */
    /* wd = (oneBy6)*fractCube - (oneBy6)*fract; */
    fractCube = fractCube - fract;
    wd = ((int16) (((int32) oneBy6 * fractCube) >> 15));

    /* Read fourth nearest value of output from the sin table */
    d = *tablePtr++;

    /* sinVal += d*wd; */
    sinVal += d * wd;


    /* Return the output value */
    return sinVal>>6;
}





//! Функция синуса, приближенная, быстрая.

//!A sine approximation via  a fourth-order cosine approx.
//! Внутри работает с точностью 16.16, но конвертируется в 8.24
//! http://www.coranac.com/2009/07/sines/
//! 30 тактов
//! \memberof V_IQmath

int32 _IQ24sinPU(int32 x) {
	int32 c, y;
	static const int32 qN = 13, qA = 12, B = 19900, C = 3516;
	x = x >> 9; //from 8.24
	c = x << (30 - qN);              // Semi-circle info into carry.
	x -= 1 << qN;                 // sine -> cosine calc

	x = x << (31 - qN);              // Mask with PI
	x = x >> (31 - qN);              // Note: SIGNED shift! (to qN)
	x = x * x >> (2 * qN - 14);          // x=x^2 To Q14

	y = B - (x * C >> 14);           // B - x^2*C
	y = (1 << qA) - (x * y >> 16);       // A - x^2*(B-x^2*C)
	y = y << 12;       // to 8.24
	return c >= 0 ? y : -y;
}

//! Функция atan2.

//!http://www.dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
//!100 тактов

//! \memberof V_IQmath
int32 _IQ24atan2PU(int32 inY , int32 inX) {
    int32 abs_inY, angle, r, r_3;

    abs_inY = labs(inY)+1;// kludge to prevent 0/0 condition

    if (inX >= 0) {
        r = _IQ24div( (inX - abs_inY), (inX + abs_inY));
        r_3=_IQmpy(_IQmpy(r,r),r);
        angle = _IQ24mpy(_IQ(0.1963f/(2.0f*_PI)),r_3)-_IQ24mpy(_IQ(0.9817f/(2.0f*_PI)),r)+_IQ24(_PI/4.0f/(2.0f*_PI));
    } else {
        r = _IQ24div( (inX + abs_inY), (abs_inY - inX));
        r_3=_IQmpy(_IQmpy(r,r),r);
        angle = _IQ24mpy(_IQ(0.1963f/(2.0f*_PI)),r_3)-_IQ24mpy(_IQ(0.9817f/(2.0f*_PI)),r)+_IQ24(3.0f*_PI/4.0f/(2.0f*_PI));
    }
    if (inY < 0) {
        angle = -angle;
    }
    angle=angle&0xFFFFFF;
    return angle;
}


//! Функция вычисления амплитуды вектора
//! 60 тактов

//! \memberof V_IQmath
int32 _IQ24mag(int32 a, int32 b) {
	return _IQ24sqrt(_IQ24mpy(a, a) + _IQ24mpy(b, b));
}

/*@}*/


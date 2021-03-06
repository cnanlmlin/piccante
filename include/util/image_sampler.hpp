/*

PICCANTE
The hottest HDR imaging library!
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

PICCANTE is free software; you can redistribute it and/or modify
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 3.0 of
the License, or (at your option) any later version.

PICCANTE is distributed in the hope that it will be useful, but
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License
( http://www.gnu.org/licenses/lgpl-3.0.html ) for more details.

*/

#ifndef PIC_UTIL_IMAGE_SAMPLER_HPP
#define PIC_UTIL_IMAGE_SAMPLER_HPP

namespace pic {

/*
			a--x----b
			|  |	|
			y--?----y
			|  |	|
			c--x----d
*/

/**
 * @brief Bilinear calculates 2D bilinear interpolation at the point (x,y).
 * @param a is the NW pixel value.
 * @param b is the NE pixel value.
 * @param c is the SW pixel value.
 * @param d is the SE pixel value.
 * @param x is the horizontal coordinate.
 * @param y is the vertical coordinate.
 * @return the evaluation of the B-spline.
 */

template<class Scalar> inline Scalar Bilinear(Scalar a, Scalar b, Scalar c, Scalar d, float x, float y)
{
    Scalar px0 = a + y * (c - a);
    Scalar px1 = b + y * (d - b);
    return px0 + x * (px1 - px0);
}

/**
 * @brief invBilinear
 * @param A
 * @param dx
 * @param dy
 * @param out
 */
inline void invBilinear(float A, float dx, float dy, float *out)
{
    dx = CLAMPi(dx, 0.0f, 1.0f);
    dy = CLAMPi(dy, 0.0f, 1.0f);

    out[0] = A * dx;
    out[1] = A * (1.0f - dx);

    float i_dy = 1.0f - dy;
    out[2] = out[0] * i_dy;
    out[3] = out[1] * i_dy;

    out[0] = out[0] * dy;
    out[1] = out[0] * dy;
}

/**
 * @brief Rx evaluates B-spline (cubic).
 * @param x is the curve parameter in [0, 1].
 * @return the evaluation of the B-spline.
 */
inline float Rx(float x)
{
    float px_1 = MAX(x - 1.0f, 0.0f);
    float px   = MAX(x,        0.0f);
    float px1  = MAX(x + 1.0f, 0.0f);
    float px2  = MAX(x + 2.0f, 0.0f);

    return (px2 * px2 * px2
            - 4.0f * px1 * px1 * px1 +
            6.0f * px * px * px
            - 4.0f * px_1 * px_1 * px_1
           ) / 6.0f;
}

} // end namespace pic

#endif /* PIC_UTIL_IMAGE_SAMPLER_HPP */


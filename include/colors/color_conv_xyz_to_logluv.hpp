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

#ifndef PIC_COLORS_COLOR_CONV_XYZ_TO_LOGLUV_HPP
#define PIC_COLORS_COLOR_CONV_XYZ_TO_LOGLUV_HPP

#include "colors/color_conv.hpp"

namespace pic {

class ColorConvXYZtoLogLuv: public ColorConv
{
protected:
    float	epsilon;

public:

    ColorConvXYZtoLogLuv()
    {
        epsilon = 1.0f;
    }

    //from XYZ to CIE LUV
    void direct(float *colIn, float *colOut)
    {

        colOut[0] = logf(colIn[1] + epsilon);

        float norm = colIn[0] + colIn[1] + colIn[2];
        float x = colIn[0] / norm;
        float y = colIn[1] / norm;

        float norm_uv = -2.0f * x + 12.0f * y + 3.0f;
        float u_prime =  4.0f * x / norm_uv;
        float v_prime =  9.0f * y / norm_uv;

        colOut[1] = u_prime;
        colOut[2] = v_prime;
    }

    //from CIE LUV to XYZ
    void inverse(float *colIn, float *colOut)
    {
        float norm = 6.0f * colIn[1] - 16.0f * colIn[2] + 12.0f;

        float x = 9.0f * colIn[1] / norm;
        float y = 4.0f * colIn[2] / norm;
        float z = 1.0f - x - y;

        float Y = MAX(expf(colIn[0]) - epsilon, 0.0f);
        norm = Y / y;

        colOut[0] = x * norm;
        colOut[1] = Y;
        colOut[2] = z * norm;
    }
};

} // end namespace pic

#endif /* PIC_COLORS_COLOR_SPACE_LOGLUV_HPP */


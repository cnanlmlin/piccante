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

#ifndef PIC_IMAGE_SAMPLERS_IMAGE_SAMPLER_BSPLINES_HPP
#define PIC_IMAGE_SAMPLERS_IMAGE_SAMPLER_BSPLINES_HPP

#include "image_samplers/image_sampler.hpp"

namespace pic {

class ImageSamplerBSplines: public ImageSampler
{
public:
    ImageSamplerBSplines() {}

    /**
     * @brief SampleImage samples an image in uniform coordiantes.
     * @param img
     * @param x
     * @param y
     * @param vOut
     */
    void SampleImage(Image *img, float x, float y, float *vOut);

    /**
     * @brief SampleImageUC
     * @param img
     * @param x
     * @param y
     * @param vOut
     */
    void SampleImageUC(Image *img, float x, float y, float *vOut);

    /**
     * @brief SampleImage samples an image in uniform coordiantes.
     * @param img
     * @param x
     * @param y
     * @param t
     * @param vOut
     */
    void SampleImage(Image *img, float x, float y, float t, float *vOut);
};

PIC_INLINE void ImageSamplerBSplines::SampleImage(Image *img, float x, float y,
        float *vOut)
{
//TODO: there's a reason for this, but I don't know it now
//	x=CLAMPi(x,0.0f,1.0f);
//	y=CLAMPi(y,0.0f,1.0f);

    float xx, yy, dx, dy;

    //Coordiantes in [0,width-1]x[0,height-1]
    x *= img->width1f;
    y *= img->height1f;
    
    //Coordinates without fractions
    xx = floorf(x);
    yy = floorf(y);
    
    //Interpolation values
    dx = (x - xx);
    dy = (y - yy);
    
    //Integer coordinates
    int ix = int(xx);
    int iy = int(yy);

    int ex, ey;

    for(int k = 0; k < img->channels; k++) {
        vOut[k] = 0.0f;
    }

    //BSplines interpolation
    float rx, ry;

    for(int j = 0; j < 4; j++) {
        ry = Rx(float(j) - 1.0f - dy);
        ey = CLAMP(iy + j, img->height);

        for(int i = 0; i < 4; i++) {
            rx = Rx(float(i) - 1.0f - dx) * ry;
            ex = CLAMP(ix + i, img->width);
            int ind = (ey * img->width + ex) * img->channels;

            for(int k = 0; k < img->channels; k++) {
                vOut[k] += img->data[ind + k] * rx;
            }
        }
    }
}

PIC_INLINE void ImageSamplerBSplines::SampleImage(Image *img, float x, float y,
        float t, float *vOut)
{
}

} // end namespace pic

#endif /* PIC_IMAGE_SAMPLERS_IMAGE_SAMPLER_BSPLINES_HPP */


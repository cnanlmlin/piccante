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

#ifndef PIC_FILTERING_FILTER_ANISOTROPIC_DIFFUSION_HPP
#define PIC_FILTERING_FILTER_ANISOTROPIC_DIFFUSION_HPP

#include "filtering/filter.hpp"
#include "filtering/filter_iterative.hpp"

namespace pic {

class FilterAnsiotropicDiffusion: public Filter
{
protected:
    //Process in a box
    void ProcessBBox(ImageRAW *dst, ImageRAWVec src, BBox *box);

    float			k, delta_t;
    unsigned int	mode;

public:
    //Basic constructor
    FilterAnsiotropicDiffusion(float k, unsigned int mode);

    static ImageRAW *AnisotropicDiffusion(ImageRAWVec imgIn, ImageRAW *imgOut,
                                          float k, unsigned int mode, unsigned int iterations)
    {
        FilterAnsiotropicDiffusion ansio_flt(k, mode);
        FilterIterative iter_flt(&ansio_flt, iterations);
        imgOut = iter_flt.Process(imgIn, imgOut);
        return imgOut;
    }

    static ImageRAW *AnisotropicDiffusion(ImageRAWVec imgIn, ImageRAW *imgOut,
                                          float sigma_s, float sigma_r)
    {

        if(sigma_s <= 0.0f) {
            sigma_s = 1.0f;
        }

        if(sigma_r <= 0.0f) {
            sigma_r = 0.05f;
        }

        unsigned int iterations = int(ceilf(5.0f * sigma_s));

        FilterAnsiotropicDiffusion ansio_flt(sigma_r, 1);
        FilterIterative iter_flt(&ansio_flt, iterations);
        imgOut = iter_flt.Process(imgIn, imgOut);
        return imgOut;
    }

};

//Basic constructor
FilterAnsiotropicDiffusion::FilterAnsiotropicDiffusion(float k,
        unsigned int mode)
{
    if(k <= 0.0f) {
        k = 0.11f;
    }

    if(mode > 2 || mode < 1) {
        mode = 1;
    }

    delta_t = 1.0f / 7.0f;

    this->k = k;
    this->mode = mode;
}

//Process in a box
void FilterAnsiotropicDiffusion::ProcessBBox(ImageRAW *dst, ImageRAWVec src,
        BBox *box)
{
    //Filtering
    ImageRAW *img = src[0];
    int channels = img->channels;

    float *gN = new float [channels];
    float *gS = new float [channels];
    float *gW = new float [channels];
    float *gE = new float [channels];

    float k2 = k * k;

    for(int j = box->y0; j < box->y1; j++) {
        for(int i = box->x0; i < box->x1; i++) {

            float *dst_data  = (*dst)(i  , j);
            float *img_data  = (*img)(i  , j);

            for(int p = 0; p < channels; p++) {
                dst_data[p] = img_data[p];
            }

            float *img_dataN = (*img)(i + 1, j);
            float *img_dataS = (*img)(i - 1, j);
            float *img_dataW = (*img)(i  , j - 1);
            float *img_dataE = (*img)(i  , j + 1);

            float cN = 0.0f;
            float cS = 0.0f;
            float cW = 0.0f;
            float cE = 0.0f;

            for(int p = 0; p < channels; p++) {
                gN[p] = img_dataN[p] - img_data[p];
                gS[p] = img_dataS[p] - img_data[p];
                gW[p] = img_dataW[p] - img_data[p];
                gE[p] = img_dataE[p] - img_data[p];

                cN += gN[p] * gN[p];
                cS += gS[p] * gS[p];
                cW += gW[p] * gW[p];
                cE += gE[p] * gE[p];
            }

            if(mode == 1) {
                cN = expf(-cN / k2);
                cS = expf(-cS / k2);
                cW = expf(-cW / k2);
                cE = expf(-cE / k2);
            }

            if(mode == 2) {
                cN = 1.0f / (1.0f + cN / k2);
                cS = 1.0f / (1.0f + cS / k2);
                cW = 1.0f / (1.0f + cW / k2);
                cE = 1.0f / (1.0f + cE / k2);
            }

            for(int p = 0; p < channels; p++) {
                dst_data[p] += delta_t *(cN * gN[p] + cS * gS[p] + cW * gW[p] + cE * gE[p]);
            }
        }
    }
}

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_ANISOTROPIC_DIFFUSION_HPP */


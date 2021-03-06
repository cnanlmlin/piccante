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

#ifndef PIC_FILTERING_FILTER_DOWNSAMPLER_2D_HPP
#define PIC_FILTERING_FILTER_DOWNSAMPLER_2D_HPP

#include "filtering/filter_npasses.hpp"
#include "filtering/filter_sampler_1d.hpp"
#include "image_samplers/image_sampler_nearest.hpp"
#include "image_samplers/image_sampler_gaussian.hpp"

namespace pic {

/**
 * @brief The FilterDownSampler2D class
 */
class FilterDownSampler2D: public FilterNPasses
{
protected:
    ImageSamplerGaussian	*isg[2];
    FilterSampler1D			*flt[2];

    bool                    swh;
    float					scaleX, scaleY;
    int						width, height;

public:

    /**
     * @brief FilterDownSampler2D
     * @param scaleX
     * @param scaleY
     */
    FilterDownSampler2D(float scaleX, float scaleY);

    /**
     * @brief FilterDownSampler2D
     * @param width
     * @param height
     */
    FilterDownSampler2D(int width, int height);

    ~FilterDownSampler2D();

    /**
     * @brief Destroy
     */
    void Destroy();
    
    /**
     * @brief PreProcess
     * @param imgIn
     * @param imgOut
     */
    void PreProcess(ImageRAWVec imgIn, ImageRAW *imgOut);

    /**
     * @brief Execute
     * @param imgIn
     * @param imgOut
     * @param width
     * @param height
     * @return
     */
    static ImageRAW *Execute(ImageRAW *imgIn, ImageRAW *imgOut, int width,
                             int height)
    {
        FilterDownSampler2D flt(width, height);
        return flt.ProcessP(Single(imgIn), imgOut);
    }

    /**
     * @brief Execute
     * @param imgIn
     * @param imgOut
     * @param scaleX
     * @param scaleY
     * @return
     */
    static ImageRAW *Execute(ImageRAW *imgIn, ImageRAW *imgOut, float scaleX,
                             float scaleY)
    {
        FilterDownSampler2D flt(scaleX, scaleY);
        return flt.ProcessP(Single(imgIn), imgOut);
    }

    /**
     * @brief Execute
     * @param imgIn
     * @param imgOut
     * @param scaleXY
     * @return
     */
    static ImageRAW *Execute(ImageRAW *imgIn, ImageRAW *imgOut, float scaleXY)
    {
        FilterDownSampler2D flt(scaleXY, scaleXY);
        return flt.ProcessP(Single(imgIn), imgOut);
    }
};

PIC_INLINE FilterDownSampler2D::FilterDownSampler2D(float scaleX, float scaleY = -1.0f)
{
    this->scaleX = scaleX;

    if(scaleY > 0.0f) {
        this->scaleY = scaleY;
    } else {
        this->scaleY = scaleX;
    }

    width  = -1;
    height = -1;

    flt[X_DIRECTION] = NULL;
    flt[Y_DIRECTION] = NULL;

    isg[X_DIRECTION] = new ImageSamplerGaussian(1.0f / (5.0f * scaleX), X_DIRECTION);
    isg[Y_DIRECTION] = new ImageSamplerGaussian(1.0f / (5.0f * scaleY), Y_DIRECTION);

    swh = true;
}

PIC_INLINE FilterDownSampler2D::FilterDownSampler2D(int width, int height)
{
    this->width  = width;
    this->height = height;

    flt[X_DIRECTION] = NULL;
    flt[Y_DIRECTION] = NULL;

    isg[X_DIRECTION] = new ImageSamplerGaussian();
    isg[Y_DIRECTION] = new ImageSamplerGaussian();

    swh = false;
}

PIC_INLINE FilterDownSampler2D::~FilterDownSampler2D()
{
    for(unsigned int i=0; i<2; i++) {
        if(flt[i] != NULL) {
            delete flt[i];
        }

        if(isg[i] != NULL) {
            delete isg[i];
        }
    }
}

PIC_INLINE void FilterDownSampler2D::PreProcess(ImageRAWVec imgIn,
        ImageRAW *imgOut)
{
    if(!swh) {
        scaleX = float(width)  / imgIn[0]->widthf;
        scaleY = float(height) / imgIn[0]->heightf;

        isg[X_DIRECTION]->Update(1.0f / (5.0f * scaleX), X_DIRECTION);
        isg[Y_DIRECTION]->Update(1.0f / (5.0f * scaleY), Y_DIRECTION);
    }

    if(flt[X_DIRECTION] == NULL) {
        flt[X_DIRECTION] = new FilterSampler1D(scaleX, X_DIRECTION, isg[X_DIRECTION]);
    } else {
        flt[X_DIRECTION]->Update(scaleX, X_DIRECTION, isg[X_DIRECTION]);
    }

    if(flt[Y_DIRECTION] == NULL) {
        flt[Y_DIRECTION] = new FilterSampler1D(scaleY, Y_DIRECTION, isg[Y_DIRECTION]);
    } else {
        flt[Y_DIRECTION]->Update(scaleY, Y_DIRECTION, isg[Y_DIRECTION]);
    }

    InsertFilter(flt[X_DIRECTION]);
    InsertFilter(flt[Y_DIRECTION]);
}


} // end namespace pic

#endif /* PIC_FILTERING_FILTER_DOWNSAMPLER_2D_HPP */


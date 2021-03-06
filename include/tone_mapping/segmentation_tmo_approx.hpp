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

#ifndef PIC_TONE_MAPPING_SEGMENTATION_TMO_APPROX_HPP
#define PIC_TONE_MAPPING_SEGMENTATION_TMO_APPROX_HPP

#include "filtering/filter_luminance.hpp"
#include "filtering/filter_iterative.hpp"
#include "filtering/filter_bilateral_2ds.hpp"

namespace pic {

class Segmentation
{
protected:
    FilterIterative			*fltIt;
    FilterBilateral2DS		*fltBil;
    ImageRAW				*L, *imgIn_flt;

    int						iterations;
    float					perCent, nLayer;

public:
    float					minVal, maxVal;

    Segmentation()
    {
        nLayer = 0.0f;
        iterations = 0;

        fltBil = NULL;
        fltIt  = NULL;

        L			= NULL;
        imgIn_flt	= NULL;

        maxVal = FLT_MAX;
        minVal = 0.0f;

        perCent  = 0.005f;
    }

    ~Segmentation()
    {
        if(imgIn_flt != NULL) {
            delete imgIn_flt;
        }

        if(L != NULL) {
            delete L;
        }

        if(fltIt != NULL) {
            delete fltIt;
        }

        if(fltBil != NULL) {
            delete fltBil;
        }
    }

    void ComputeStatistics(ImageRAW *imgIn)
    {
        float nLevels, area;

        nLevels		= log10f(maxVal) - log10f(minVal) + 1.0f;
        nLayer		= ((maxVal - minVal) / nLevels) / 4.0f;
        area		= imgIn->widthf * imgIn->heightf * perCent;
        iterations	= MAX(int(sqrtf(area)) / 2, 1);
    }

    ImageRAW *SegmentationBilatearal(ImageRAW *imgIn)
    {
        ComputeStatistics(imgIn);

        //Create filters
        if(fltIt == NULL) {
            fltBil = new FilterBilateral2DS(1.0f, nLayer);
            fltIt  = new FilterIterative(fltBil, iterations);
        }

#ifdef PIC_DEBUG
        printf("Layer: %f iterations: %d\n", nLayer, iterations);
#endif
        //Iterative bilateral filtering
        ImageRAW *imgOut = fltIt->ProcessP(Single(imgIn), imgIn_flt);

        //imgOut->Write("filtered.pfm");
        return imgOut;
    }

    ImageRAW *SegmentationSuperPixels(ImageRAW *imgIn, int nSuperPixels = 4096)
    {
        Slic sp;
        sp.Process(imgIn, nSuperPixels);
        ImageRAW *imgOut = sp.getMeanImage(NULL);
        return imgOut;
    }

    ImageRAW *Compute(ImageRAW *imgIn, ImageRAW *imgOut)
    {
        if(imgIn == NULL) {
            return NULL;
        }

        if(!imgIn->isValid() || (imgIn->channels != 3)) {
            return NULL;
        }

        if(imgOut == NULL) {
            imgOut = new ImageRAW(1, imgIn->width, imgIn->height, 1);
        }

        //Compute luminance
        FilterLuminance::Execute(imgIn, imgOut, LT_CIE_LUMINANCE);

        //Get min and max value
        maxVal = imgOut->getMaxVal()[0];
        minVal = imgOut->getMinVal()[0] + 1e-9f;

        ImageRAW *imgIn_flt = SegmentationBilatearal(imgIn);

        //Thresholding
        float minShift = floorf(log10f(minVal));
        float *data = imgIn_flt->data;

        #pragma omp parallel for

        for(int i = 0; i < imgIn_flt->size(); i += imgIn_flt->channels) {
            float Lum = 0.213f * data[i] + 0.715f * data[i + 1] + 0.072f * data[i + 2];

            if(Lum > 0.0f) {
                imgOut->data[i / 3] = floorf(log10f(Lum));
            } else {
                imgOut->data[i / 3] = minShift;
            }
        }

        //imgOut->Write("Segmentation.pfm");

        return imgOut;
    }
};

} // end namespace pic

#endif /* PIC_TONE_MAPPING_SEGMENTATION_TMO_APPROX_HPP */


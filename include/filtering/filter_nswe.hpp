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

#ifndef PIC_FILTERING_FILTER_NSWE_HPP
#define PIC_FILTERING_FILTER_NSWE_HPP

#include "filtering/filter.hpp"

namespace pic {

class FilterNSWE: public Filter
{
protected:
    //Process in a box
    void ProcessBBox(ImageRAW *dst, ImageRAWVec src, BBox *box)
    {
        //Filtering
        ImageRAW *img = src[0];
        int channels = img->channels;

        for(int j = box->y0; j < box->y1; j++) {

            for(int i = box->x0; i < box->x1; i++) {

                float *dst_data   = (*dst)(i  , j);

                float *img_data  = (*img)(i    , j);
                float *img_dataN = (*img)(i + 1, j);
                float *img_dataS = (*img)(i - 1, j);
                float *img_dataW = (*img)(i    , j - 1);
                float *img_dataE = (*img)(i    , j + 1);

                for(int k = 0; k < channels; k++) {
                    int tmp = k * 4;
                    dst_data[tmp    ] = img_dataN[k] - img_data[k];
                    dst_data[tmp + 1] = img_dataS[k] - img_data[k];
                    dst_data[tmp + 2] = img_dataW[k] - img_data[k];
                    dst_data[tmp + 3] = img_dataE[k] - img_data[k];
                }
            }
        }
    }

    ImageRAW *SetupAux(ImageRAWVec imgIn, ImageRAW *imgOut)
    {
        if(imgOut == NULL) {
            imgOut = new ImageRAW(1, imgIn[0]->width, imgIn[0]->height,
                                  4 * imgIn[0]->channels);
        }

        return imgOut;
    }

public:
    //Basic constructor
    FilterNSWE() {};

    /**Output size*/
    void OutputSize(ImageRAW *imgIn, int &width, int &height, int &channels, int &frames)
    {
        width       = imgIn->width;
        height      = imgIn->height;
        channels    = imgIn->channels * 4;
        frames      = imgIn->frames;
    }

    //Filtering
    static ImageRAW *Execute(ImageRAW *imgIn, ImageRAW *imgOut)
    {
        FilterNSWE filter;
        return filter.ProcessP(Single(imgIn), imgOut);
    }

    //Filtering
    static ImageRAW *Execute(std::string fileInput, std::string fileOutput)
    {
        ImageRAW imgIn(fileInput);
        ImageRAW *out = FilterNSWE::Execute(&imgIn, NULL);
        out->Write(fileOutput);
        return out;
    }
};

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_NSWE_HPP */


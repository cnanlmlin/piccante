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

#ifndef PIC_FEATURES_MATCHING_SUSAN_CORNER_DETECTOR_HPP
#define PIC_FEATURES_MATCHING_SUSAN_CORNER_DETECTOR_HPP

#include "util/vec.hpp"

#include "image_raw.hpp"
#include "filtering/filter_luminance.hpp"

#include "features_matching/general_corner_detector.hpp"

#ifndef PIC_DISABLE_EIGEN
#include "externals/Eigen/Dense"
#endif
namespace pic {

#ifndef PIC_DISABLE_EIGEN

class SusanCornerDetector: public GeneralCornerDetector
{
protected:
    ImageRAW *lum_flt;
    bool      bComputeThreshold;

    float     sigma, threshold;
    int       radius, radius_maxima;

public:
    SusanCornerDetector() : GeneralCornerDetector()
    {
        lum_flt = NULL;

        bComputeThreshold = true;
        Update();
    }

    ~SusanCornerDetector()
    {
        if(bLum) {
            delete lum;
        }

        if(lum_flt != NULL) {
            delete lum_flt;
        }
    }

    void Update(float sigma = 1.0f, int radius_maxima = 5, int radius = 3, float threshold = 0.001f)
    {
        if(sigma > 0.0f) {
            this->sigma = sigma;
        } else {
            this->sigma = 1.0f;
        }

        if(radius > 0) {
            this->radius = radius;
        } else {
            this->radius = 3;
        }

        if(threshold > 0.0f) {
            this->threshold = threshold;
        } else {
            this->threshold = 0.001f;
        }

        if(radius_maxima > 0){
            this->radius_maxima = radius_maxima;
        } else {
            this->radius_maxima = 5;
        }
    }

    void Compute(ImageRAW *img, std::vector< Eigen::Vector3f > *corners)
    {
        if(img == NULL) {
            return;
        }

        if(img->channels == 1) {
            bLum = false;
            lum = img;
        } else {
            bLum = true;
            lum = FilterLuminance::Execute(img, lum, LT_CIE_LUMINANCE);
        }

        if(corners == NULL) {
            corners = new std::vector< Eigen::Vector3f >;
        }

        corners->clear();

        //Filtering the image
        FilterGaussian2D flt(sigma);
        lum_flt = flt.ProcessP(Single(lum), NULL);

        //"rasterizing" a circle
        std::vector< int > x, y;
        int radius_2 = radius * radius;
        for(int i=-radius; i<=radius; i++) {
            int i_2 = i * i;
            for(int j=-radius; j<=radius; j++) {

                if((j == 0) && (i == 0)) {
                    continue;
                }

                int r_2 = i_2 + j * j;
                if(r_2 <= radius_2){
                    x.push_back(j);
                    y.push_back(i);
                }
            }
        }

        int width  = lum_flt->width;
        int height = lum_flt->height;

        float C = float(x.size());

        float t = 0.05f; //depends on image noise

        float g = C * 0.5f; //Geometric constant for determing corners

        ImageRAW R(1,width, height, 1);
        R.SetZero();
        for(int i=radius; i<(height - radius - 1); i++) {
            for(int j=radius; j<(width - radius - 1); j++) {

                int ind = i * width + j;

                float sum = 0.0f;

                for(unsigned int k=0; k<x.size(); k++) {
                    int ind_c = (i + y[k]) * width + (j + x[k]);

                    float diff = (lum_flt->data[ind_c] - lum_flt->data[ind]) / t;
                    float diff_2 = diff * diff;
                    float diff_4 = diff_2 * diff_2;
                    float diff_6 = diff_4 * diff_2;

                    sum += expf(-diff_6);
                }

                if(sum < g) {
                    R.data[ind] = g - sum;
                }
            }
        }

        int side = radius_maxima * 2 + 1;
        int *indices = new int [side * side];

        for(int i=radius_maxima; i<(height - radius_maxima - 1); i++) {

            int tmp = i * width;

            for(int j=radius_maxima; j<(width - radius_maxima - 1); j++) {
                int ind = tmp + j;

                if(R.data[ind] <= 0.0f) {
                    continue;
                }

                indices[0] = ind;
                int counter = 1;

                for(int k=-radius_maxima; k<=radius_maxima; k++) {
                    int yy = CLAMP(i + k, height);

                    for(int l=-radius_maxima; l<=radius_maxima; l++) {

                        if((l==0)&&(k==0)) {
                            continue;
                        }

                        int xx = CLAMP(j + l, width);

                        ind = yy * width + xx;

                        if(R.data[ind]>0.0f){
                            indices[counter] = ind;
                            counter++;
                        }

                    }
                }

                //are other corners near-by?
                if(counter > 1) {
                    //finding the maximum value
                    float R_value = R.data[indices[0]];
                    int index = 0;

                    for(int k=1; k<counter; k++){
                        if(R.data[indices[k]] > R_value) {
                            R_value = R.data[indices[k]];
                            index = k;
                        }
                    }

                    if(index == 0){
                        corners->push_back(Eigen::Vector3f (float(j), float(i), 1.0f) );
                    }
                } else {
                    corners->push_back(Eigen::Vector3f (float(j), float(i), 1.0f) );
                }
            }
        }
    }
\
};

#endif

} // end namespace pic

#endif /* PIC_FEATURES_MATCHING_SUSAN_CORNER_DETECTOR_HPP */


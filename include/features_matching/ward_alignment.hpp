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

#ifndef PIC_FEATURES_MATCHING_WARD_ALIGNMENT_HPP
#define PIC_FEATURES_MATCHING_WARD_ALIGNMENT_HPP

#include <vector>

#include "image_raw.hpp"
#include "image_samplers/image_sampler_bilinear.hpp"
#include "filtering/filter_downsampler_2D.hpp"
#include "filtering/filter_luminance.hpp"

#ifndef PIC_DISABLE_EIGEN
#include "externals/Eigen/Dense"
#endif

namespace pic {

#ifndef PIC_DISABLE_EIGEN

/**
 * @brief The WardAlignment class
 */
class WardAlignment
{
protected:
    float tolerance, percentile;

public:
    ImageRAWVec             img1_v, img2_v, luminance;
    std::vector< bool* >    tb1_v, tb2_v, eb2_shifted_v, tb2_shifted_v;

    /**
     * @brief WardAlignment
     */
    WardAlignment()
    {
        Update(0.5f, 0.015625f);
    }

    ~WardAlignment()
    {
        for(unsigned int i=0; i<luminance.size(); i++) {
            delete luminance[i];
        }

        for(unsigned int i=0; i<img1_v.size(); i++) {
            delete img1_v[i];
        }

        for(unsigned int i=0; i<img2_v.size(); i++) {
            delete img2_v[i];
        }

        for(unsigned int i=0; i<tb1_v.size(); i++) {
            delete tb1_v[i];
        }

        for(unsigned int i=0; i<tb2_v.size(); i++) {
            delete tb2_v[i];
        }

        for(unsigned int i=0; i<eb2_shifted_v.size(); i++) {
            delete eb2_shifted_v[i];
        }

        for(unsigned int i=0; i<tb2_shifted_v.size(); i++) {
            delete tb2_shifted_v[i];
        }
    }

    /**
     * @brief Update sets parameters up for MTB
     * @param percentile
     * @param tolerance
     */
    void Update(float percentile, float tolerance)
    {
        if(percentile < 0.0f && percentile > 1.0f) {
            percentile = 0.5f;
        }

        if(tolerance > 0.0625f) {
            tolerance = 0.015625f;
        }

        this->percentile = percentile;
        this->tolerance = tolerance;
    }

    /**
     * @brief MTB computes the median threshold mask
     * @param img
     * @param L
     * @return
     */
    bool *MTB(ImageRAW *img, ImageRAW *L)
    {
        bool bDelete = (L == NULL);

        if(img->channels == 1)
        {
            bDelete = false;
            L = img;
        } else {
            L = FilterLuminance::Execute(img, L, LT_WARD_LUMINANCE);
        }

        int n = L->nPixels();
        bool *maskThr = new bool[n * 2];
        bool *maskEb = &maskThr[n];

        float medVal = L->getMedVal(percentile);

        float A = medVal - tolerance;
        float B = medVal + tolerance;

        for(int i = 0; i < n; i++) {
            maskThr[i] = L->data[i] > medVal;
            maskEb[i]  = !((L->data[i] >= A) && (L->data[i] <= B));
        }

        if(bDelete) {
            delete L;
        }

        return maskThr;
    }

    /**
     * @brief GetExpShift computes the shift vector for moving an img1 onto img2
     * @param img1
     * @param img2
     * @param shift_bits
     * @return
     */
    Eigen::Vector2i GetExpShift(ImageRAW *img1, ImageRAW *img2,
                                   int shift_bits = 6)
    {
        if(img1 == NULL || img2 == NULL) {
            return Eigen::Vector2i(0, 0.0);
        }

        if(!img1->SimilarType(img2)) {
            return Eigen::Vector2i(0, 0);
        }

        ImageRAW *L1, *L2;

        if(img1->channels == 1) {
            L1 = img1;
        } else {
            L1 = FilterLuminance::Execute(img1, NULL, LT_WARD_LUMINANCE);
            luminance.push_back(L1);
        }

        if(img2->channels == 1) {
            L2 = img2;
        } else {
            L2 = FilterLuminance::Execute(img2, NULL, LT_WARD_LUMINANCE);
            luminance.push_back(L2);
        }

        int min_coord = MIN(L1->width, L1->height);
         if(min_coord < (1 << shift_bits)) {
             shift_bits = MAX(log2(min_coord) - 1, 1);
         }

        Eigen::Vector2i cur_shift, ret_shift;

        cur_shift = Eigen::Vector2i(0, 0);
        ret_shift = Eigen::Vector2i(0, 0);

        ImageRAW *sml_img1 = NULL;
        ImageRAW *sml_img2 = NULL;

        while(shift_bits > 0) {
            float scale = powf(2.0f, float(-shift_bits));

            sml_img1 = FilterDownSampler2D::Execute(L1, NULL, scale);
            sml_img2 = FilterDownSampler2D::Execute(L2, NULL, scale);

            //tracking memory
            img1_v.push_back(sml_img1);
            img2_v.push_back(sml_img2);

            int width  = sml_img1->width;
            int height = sml_img1->height;
            int n = width * height;

             //Computing the median threshold mask
            bool *tb1 = MTB(sml_img1, NULL);
            bool *eb1  = &tb1[n];

            bool *tb2 = MTB(sml_img2, NULL);
            bool *eb2  = &tb2[n];

            //tracking memory
            tb1_v.push_back(tb1);
            tb2_v.push_back(tb2);
            
            int min_err = n;

            bool *tb2_shifted = new bool[n];
            bool *eb2_shifted = new bool[n];

            tb2_shifted_v.push_back(tb2_shifted);
            eb2_shifted_v.push_back(eb2_shifted);

            for(int i = -1; i <= 1; i++) {

                for(int j = -1; j <= 1; j++) {

                    int xs = cur_shift[0] + i;
                    int ys = cur_shift[1] + j;

                    BufferShift(tb2_shifted, tb2, xs, ys, width, height, 1, 1);
                    BufferShift(eb2_shifted, eb2, xs, ys, width, height, 1, 1);

                    int err = 0;
                    for(int k=0; k<n; k++) {
                        bool diff_b = tb1[k] ^ tb2_shifted[k];
                        diff_b = diff_b & eb1[k];
                        diff_b = diff_b & eb2_shifted[k];

                        if(diff_b) {
                            err++;
                        }
                    }

                    if(err < min_err) {
                        ret_shift[0] = xs;
                        ret_shift[1] = ys;
                        min_err = err;
                    }
                }
            }

            shift_bits--;

            cur_shift[0] = ret_shift[0] * 2;
            cur_shift[1] = ret_shift[1] * 2;
        }

        return cur_shift;
    }

    /**
     * @brief Execute aligns imgSource to imgTarget
     * @param imgTarget
     * @param imgSource
     * @param shift
     * @return
     */
    static ImageRAW *Execute(ImageRAW *imgTarget, ImageRAW *imgSource, Eigen::Vector2i &shift)
    {
        WardAlignment wa;

        if(imgTarget == NULL || imgSource == NULL) {
            return NULL;
        }

        if(!imgTarget->SimilarType(imgSource)) {
            return NULL;
        }

        ImageRAW *ret = imgTarget->AllocateSimilarOne();
        ret->SetZero();

        shift = wa.GetExpShift(imgTarget, imgSource);

        #ifdef PIC_DEBUG
            printf("Ward alignment shift: (%d, %d)\n", shift[0], shift[1]);
        #endif

        BufferShift(ret->data, imgSource->data, shift[0], shift[1], imgTarget->width,
                    imgTarget->height, imgTarget->channels, imgTarget->frames);

        return ret;
    }
};

#endif

} // end namespace pic

#endif /* PIC_FEATURES_MATCHING_WARD_ALIGNMENT_HPP */


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

#ifndef PIC_FILTERING_FILTER_BILATERAL_2DS_HPP
#define PIC_FILTERING_FILTER_BILATERAL_2DS_HPP

#include <random>

#include "filtering/filter.hpp"
#include "util/precomputed_gaussian.hpp"
#include "point_samplers/sampler_random_m.hpp"

namespace pic {

class FilterBilateral2DS: public Filter
{
protected:
    float					sigma_s, sigma_r;

    MRSamplers<2>			*ms;

    int						halfSizeKernel;
    PrecomputedGaussian		*pg;

    //Process in a box
    void ProcessBBox(ImageRAW *dst, ImageRAWVec src, BBox *box);

public:
    int nSamples;

    //Basic constructors
    FilterBilateral2DS()
    {
        pg = NULL;
        ms = NULL;
    }
    FilterBilateral2DS(std::string nameFile, float sigma_r);

    //Constructors using Init
    FilterBilateral2DS(SAMPLER_TYPE type, float sigma_s, float sigma_r, int mult);
    FilterBilateral2DS(float sigma_s, float sigma_r, int mult);
    FilterBilateral2DS(float sigma_s, float sigma_r);

    //Init
    void Init(SAMPLER_TYPE type, float sigma_s, float sigma_r, int mult);

    std::string Signature()
    {
        return GenBilString("S", sigma_s, sigma_r);
    }

    //Set sigma_r
    void SetSigma_r(float sigma_r)
    {
        this->sigma_r = sigma_r;
    }

    //Write the kernel
    bool Write(std::string filename);

    //Read the kernel
    bool Read(std::string filename);

    //Filtering
    static ImageRAW *Execute(ImageRAW *imgIn,
                             float sigma_s, float sigma_r)
    {
        //Filtering
        FilterBilateral2DS filter(sigma_s, sigma_r, 1);
        //long t0 = timeGetTime();
        ImageRAW *imgOut = filter.ProcessP(Single(imgIn), NULL);
        //long t1 = timeGetTime();
        //printf("Stochastic Bilateral Filter time: %ld\n",t1-t0);
        return imgOut;
    }

    //Filtering
    static ImageRAW *Execute(ImageRAW *imgIn, ImageRAW *imgEdge,
                             float sigma_s, float sigma_r)
    {
        //Filtering
        FilterBilateral2DS filter(sigma_s, sigma_r, 1);
        //long t0 = timeGetTime();
        ImageRAW *imgOut;

        if(imgEdge == NULL) {
            imgOut = filter.ProcessP(Single(imgIn), NULL);
        } else {
            imgOut = filter.ProcessP(Double(imgIn, imgEdge), NULL);
        }

        //long t1 = timeGetTime();
        //printf("Stochastic Bilateral Filter time: %ld\n",t1-t0);
        return imgOut;
    }

    static ImageRAW *Execute(std::string nameIn,
                             std::string nameOut,
                             float sigma_s, float sigma_r, SAMPLER_TYPE type = ST_BRIDSON, int mult = 1)
    {
        //Load the image
        ImageRAW imgIn(nameIn);

        //Filtering
        FilterBilateral2DS filter(type, sigma_s, sigma_r, mult);
        //long t0 = timeGetTime();
        ImageRAW *imgOut = filter.ProcessP(Single(&imgIn), NULL);
        //long t1 = timeGetTime();
        //printf("Stochastic Bilateral Filter time: %ld\n",t1-t0);

        //Write image out
        imgOut->Write(nameOut);
        return imgOut;
    }

    //Filtering
    static ImageRAW *Execute(std::string nameIn,
                             std::string nameIn2,
                             std::string nameOut,
                             float sigma_s, float sigma_r, SAMPLER_TYPE type = ST_BRIDSON, int mult = 1)
    {
        //Load the image
        ImageRAW imgIn(nameIn);
        ImageRAW imgIn2(nameIn2);

        //Filtering
        FilterBilateral2DS filter(type, sigma_s, sigma_r, mult);
        //long t0 = timeGetTime();
        ImageRAW *imgOut = filter.ProcessP(Double(&imgIn, &imgIn2), NULL);
        //long t1 = timeGetTime();
        //printf("Stochastic Bilateral Filter time: %ld\n",t1-t0);

        //Write image out
        imgOut->Write(nameOut);
        return imgOut;
    }

    static inline float BilateralStoK(int kernelSize)
    {
        //	float ret = 0.9577f/(0.6466f*float(kernelSize)-0.9175f)+0.4505;
        float ret = 0.4055f / (0.6437f * float(kernelSize) - 1.1083f) + 0.7347f;

        if(ret < 0.0f) {
            return 3.0f;
        } else {
            return ret;
        }
    }

    static inline float BilateralStoK2(int kernelSize)
    {
        float ret = 0.3233f / (0.5053f * float(kernelSize) - 0.8272f) + 0.7366f;

        if(ret < 0.0f) {
            return 2.5f;
        } else {
            return ret;
        }
    }

    /**Precomputing kernels*/
    static void PrecomputedKernels()
    {
        char nameFile[512];

        for(int i = 0; i < 6; i++) {
            float sigma_s = powf(2.0f, float(i));

            int nSamples = PrecomputedGaussian::KernelSize(sigma_s);
            int nSamplesDiv2  = nSamples / 2;
            int nMaxSamples = nSamplesDiv2 * nSamplesDiv2;
            int oldNSamples = -1;

            printf("Computing kernel sigma_s: %f\n", sigma_s);

            for(int j = 1; j <= 16; j++) {
                printf("Multiplier: %d\n", j);
                nSamples = MIN((nSamplesDiv2 * j), nMaxSamples);

                if(nSamples == oldNSamples) {
                    break;
                }

                FilterBilateral2DS f2DS(sigma_s, 0.01f, j);

                sprintf(nameFile, "kernel_%3.2f_%d.txt", sigma_s, j);
                f2DS.Write(nameFile);
                oldNSamples = nSamples;
            }
        }
    }
};

//Basic constructors
PIC_INLINE FilterBilateral2DS::FilterBilateral2DS(std::string nameFile,
        float sigma_r)
{
    Read(nameFile);
    this->sigma_r = sigma_r;
}

//Init constructors
PIC_INLINE FilterBilateral2DS::FilterBilateral2DS(SAMPLER_TYPE type,
        float sigma_s, float sigma_r, int mult)
{
    Init(type, sigma_s, sigma_r, mult);
}

PIC_INLINE FilterBilateral2DS::FilterBilateral2DS(float sigma_s, float sigma_r,
        int mult)
{
    Init(ST_BRIDSON, sigma_s, sigma_r, mult);
}

PIC_INLINE FilterBilateral2DS::FilterBilateral2DS(float sigma_s, float sigma_r)
{
    Init(ST_BRIDSON, sigma_s, sigma_r, 1);
}

//Init
PIC_INLINE void FilterBilateral2DS::Init(SAMPLER_TYPE type, float sigma_s,
        float sigma_r, int mult)
{
    //protected values are assigned/computed
    this->sigma_s = sigma_s;
    this->sigma_r = sigma_r;

    //Precomputation of the Gaussian Kernel
    pg = new PrecomputedGaussian(sigma_s);//, sigma_r);

    //Poisson samples
    int nMaxSamples = pg->halfKernelSize * pg->halfKernelSize;

    int nSamples = int(lround(float(pg->kernelSize)) * BilateralStoK(int(sigma_s))) * mult;

    nSamples = MIN(nSamples, nMaxSamples);
#ifdef PIC_DEBUG
    printf("Nsamples: %d %f\n", nSamples, sigma_s);
#endif
//	nSamples = MIN(	(pg->halfKernelSize*mult),nMaxSamples);

    ms = new MRSamplers<2>(type, pg->halfKernelSize, nSamples, 1, 64);
}

//Process in a box
PIC_INLINE void FilterBilateral2DS::ProcessBBox(ImageRAW *dst, ImageRAWVec src,
        BBox *box)
{
    //Filtering
    double Gauss1, Gauss2;
    double I_val[4], tmpC[4];
    double  tmp, tmp2, tmp3, sum;

    ImageRAW *edge, *base, *selector;
    selector = NULL;

    int nImg = src.size();

    switch(nImg) {
    //Bilateral filter
    case 1:
        base = src[0];
        edge = src[0];
        break;

    //Cross bilateral filter
    case 2:
        base = src[0];
        edge = src[1];
        break;

    case 3:
        base = src[0];
        edge = src[1];
        selector = src[2];
        break;

    default:
        base = src[0];
        edge = src[0];
    }

    int width = dst->width;
    int height = dst->height;
    int channels = dst->channels;
    int edgeChannels = edge->channels;

    double sigma_r2 = sigma_r * sigma_r * 2.0;
    bool sumTest;

    RandomSampler<2> *ps;
    int nSamples;

    //Mersenne Twister
    std::mt19937 m(rand() % 10000);

    for(int j = box->y0; j < box->y1; j++) {
        for(int i = box->x0; i < box->x1; i++) {
            float *dst_data  = (*dst )(i, j);
            float *base_data = (*base)(i, j);
            float *edge_data = (*edge)(i, j);

            for(int l = 0; l < edgeChannels; l++) {
#ifdef PIC_SELECTOR

                if(selector != NULL) {
                    float t = MIN(MAX(selector->data[c], 0.0f), 1.0f);
                    float val = t * base->data[c + l] + (1.0f - t) * edge->data[c + l];
                    I_val[l] = val;
                } else
#endif
                    I_val[l] = edge_data[l];
            }

            for(int l = 0; l < channels; l++) {
                tmpC[l] = 0.0;
            }

            sum = 0.0;

            ps = ms->getSampler(&m);
            nSamples = ps->samplesR.size();

            for(int k = 0; k < nSamples; k += 2) {
                //Spatial Gaussian kernel
                Gauss1 =	pg->coeff[ps->samplesR[k  ] + pg->halfKernelSize] *
                            pg->coeff[ps->samplesR[k + 1] + pg->halfKernelSize];

                //Address
                int ci = CLAMP(i + ps->samplesR[k  ], width);
                int cj = CLAMP(j + ps->samplesR[k + 1], height);

                float *edge_data = (*edge)(ci, cj);

                //Range Gaussian Kernel
                tmp = 0.0;

                for(int l = 0; l < edgeChannels; l++) {
#ifdef PIC_SELECTOR

                    if(selector != NULL) {
                        float t = MIN(MAX(selector->data[c2], 0.0f), 1.0f);
                        float val = t * base->data[c2 + l] + (1.0f - t) * edge->data[c2 + l];
                        tmp3 = val - I_val[l];
                    } else
#endif
                        tmp3 = edge_data[l] - I_val[l];

                    tmp += tmp3 * tmp3;
                }

                Gauss2 = exp(-tmp / sigma_r2);

                //Weight
                tmp2 = Gauss1 * Gauss2;
                sum += tmp2;

                float *base_data = (*base)(ci, cj);

                //Filtering
                for(int l = 0; l < channels; l++) {
#ifdef PIC_SELECTOR

                    if(selector != NULL) {
                        float t = MIN(MAX(selector->data[c2], 0.0f), 1.0f);
                        float val = t * base->data[c2 + l] + (1.0f - t) * edge->data[c2 + l];
                        tmpC[l] += val * tmp2;
                    } else
#endif
                        tmpC[l] += base_data[l] * tmp2;
                }
            }

            //Normalization
            sumTest = sum > 0.0;

            for(int l = 0; l < channels; l++)
#ifdef PIC_SELECTOR
                if(selector != NULL) {
                    float t = MIN(MAX(selector->data[c], 0.0f), 1.0f);
                    float val = t * base->data[c + l] + (1.0f - t) * edge->data[c + l];
                    dst->data[c + l] = sumTest ? tmpC[l] / sum : val;
                } else
#endif
                    dst_data[l] = sumTest ? float(tmpC[l] / sum) : base_data[l];
        }
    }
}

//Write the kernel
PIC_INLINE bool FilterBilateral2DS::Write(std::string nameFile)
{
    //TODO: add the writing of (sigms_s, sigma_r)
    return ms->Write(nameFile);
}

//Read the kernel
PIC_INLINE bool FilterBilateral2DS::Read(std::string filename)
{
    //TODO: add the reading of (sigms_s, sigma_r)
    //Precomputation of the Gaussian Kernel
    pg = new PrecomputedGaussian(sigma_s);

    if( ms != NULL) {
        delete ms;
    }
    ms = new MRSamplers<2>();
    return ms->Read(filename);
}

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_BILATERAL_2DS_HPP */


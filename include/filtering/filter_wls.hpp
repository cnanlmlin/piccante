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

#ifndef PIC_FILTERING_FILTER_WLS_HPP
#define PIC_FILTERING_FILTER_WLS_HPP

#include "filtering/filter.hpp"

#ifndef PIC_DISABLE_EIGEN

#include "externals/Eigen/Sparse"
#include "externals/Eigen/src/SparseCore/SparseMatrix.h"

namespace pic {

class FilterWLS: public Filter
{
protected:
    /**WLSFilter: smoothing WLS filter for gray-scale images*/
    ImageRAW *SingleChannel(ImageRAWVec imgIn, ImageRAW *imgOut)
    {
        ImageRAW *L = imgIn[0];

        int width  = L->width;
        int height = L->height;
        int tot    = height * width;

        Eigen::VectorXd b, x;
        b = Eigen::VectorXd::Zero(tot);

        #ifdef PIC_DEBUG
            printf("Init matrix...");
        #endif

        std::vector< Eigen::Triplet< double > > tL;

        for(int i = 0; i < height; i++) {
            int tmpInd = i * width;

            for(int j = 0; j < width; j++) {

                float Ltmp, tmp;
                int indJ;
                int indI = tmpInd + j;
                float Lref = L->data[indI];

                b[indI] = Lref;

                float sum = 0.0f;

                if((i - 1) >= 0) {
                    indJ = indI - width;
                    Ltmp = L->data[indJ];
                    tmp  = -lambda / (powf(fabsf(Ltmp - Lref), alpha) + epsilon);
                    tL.push_back(Eigen::Triplet< double > (indI, indJ, tmp));
                    sum += tmp;
                }

                if((i + 1) < height) {
                    indJ = indI + width;
                    Ltmp = L->data[indJ];
                    tmp  = -lambda / (powf(fabsf(Ltmp - Lref), alpha) + epsilon);
                    tL.push_back(Eigen::Triplet< double > (indI, indJ, tmp));
                    sum += tmp;
                }

                if((j - 1) >= 0) {
                    indJ = indI - 1;
                    Ltmp = L->data[indJ];
                    tmp  = -lambda / (powf(fabsf(Ltmp - Lref), alpha) + epsilon);
                    tL.push_back(Eigen::Triplet< double > (indI, indJ, tmp));
                    sum += tmp;
                }

                if((j + 1) < width) {
                    indJ = indI + 1;
                    Ltmp = L->data[indJ];
                    tmp  = -lambda / (powf(fabsf(Ltmp - Lref), alpha) + epsilon);
                    tL.push_back(Eigen::Triplet< double > (indI, indJ, tmp));
                    sum += tmp;
                }

                tL.push_back(Eigen::Triplet< double > (indI, indI, 1.0f - sum));
            }
        }

        #ifdef PIC_DEBUG
            printf("Ok\n");
        #endif

        Eigen::SparseMatrix<double> A = Eigen::SparseMatrix<double>(tot, tot);
        A.setFromTriplets(tL.begin(), tL.end());

        Eigen::SimplicialCholesky<Eigen::SparseMatrix<double> > solver(A);
        x = solver.solve(b);

        if(solver.info() != Eigen::Success) {
            #ifdef PIC_DEBUG
                printf("SOLVER FAILED!\n");
            #endif
            return NULL;
        }

        #ifdef PIC_DEBUG
            printf("SOLVER SUCCESS!\n");
        #endif

        #pragma omp parallel for

        for(int i = 0; i < tot; i++) {
            imgOut->data[i] = float(x(i));
        }

        return imgOut;
    }

    /**MultiChannel: smoothing WLS filter for color images*/
    ImageRAW *MultiChannel(ImageRAWVec imgIn, ImageRAW *imgOut)
    {
        ImageRAW *img = imgIn[0];

        int width  = img->width;
        int height = img->height;
        int tot    = height * width;

        alpha /= 2.0f;

        int stridex = width * img->channels;

        #ifdef PIC_DEBUG
            printf("Init matrix...");
        #endif

        std::vector< Eigen::Triplet< double > > tL;

        for(int i = 0; i < height; i++) {
            int tmpInd = i * width;

            for(int j = 0; j < width; j++) {

                float sum = 0.0f;
                float tmp;
                int indJ;
                int indI = tmpInd + j;
                int indImg = indI * img->channels;

                if((i - 1) >= 0) {
                    indJ = indImg - stridex;
                    float diff = 0.0f;

                    for(int p = 0; p < img->channels; p++) {
                        float tmpDiff = img->data[indJ + p] - img->data[indImg + p];
                        diff += tmpDiff * tmpDiff;
                    }

                    tmp  = -lambda / (powf(diff, alpha) + epsilon);

                    tL.push_back(Eigen::Triplet< double > (indI, indI - width , tmp));

                    sum += tmp;
                }

                if((i + 1) < height) {
                    indJ = indImg + stridex;
                    float diff = 0.0f;

                    for(int p = 0; p < img->channels; p++) {
                        float tmpDiff = img->data[indJ + p] - img->data[indImg + p];
                        diff += tmpDiff * tmpDiff;
                    }

                    tmp  = -lambda / (powf(diff, alpha) + epsilon);
                    tL.push_back(Eigen::Triplet< double > (indI, indI + width , tmp));
                    sum += tmp;
                }

                if((j - 1) >= 0) {
                    indJ = indImg - img->channels;
                    float diff = 0.0f;

                    for(int p = 0; p < img->channels; p++) {
                        float tmpDiff = img->data[indJ + p] - img->data[indImg + p];
                        diff += tmpDiff * tmpDiff;
                    }

                    tmp  = -lambda / (powf(diff, alpha) + epsilon);
                    tL.push_back(Eigen::Triplet< double > (indI, indI - 1 , tmp));
                    sum += tmp;
                }

                if((j + 1) < width) {
                    indJ = indImg + img->channels;
                    float diff = 0.0f;

                    for(int p = 0; p < img->channels; p++) {
                        float tmpDiff = img->data[indJ + p] - img->data[indImg + p];
                        diff += tmpDiff * tmpDiff;
                    }

                    tmp  = -lambda / (powf(diff, alpha) + epsilon);

                    tL.push_back(Eigen::Triplet< double > (indI, indI + 1 , tmp));
                    sum += tmp;
                }

                tL.push_back(Eigen::Triplet< double > (indI, indI, 1.0f - sum));
            }
        }

        #ifdef PIC_DEBUG
            printf("Ok\n");
        #endif

        Eigen::SparseMatrix<double> A = Eigen::SparseMatrix<double>(tot, tot);

        A.setFromTriplets(tL.begin(), tL.end());

        Eigen::SimplicialCholesky< Eigen::SparseMatrix< double > > solver(A);

        for(int i = 0; i < imgOut->channels; i++) {
            Eigen::VectorXd b, x;

            b = Eigen::VectorXd::Zero(tot);
            #pragma omp parallel for

            for(int j = 0; j < tot; j++) {
                b[j] = img->data[j * img->channels + i];
            }

            x = solver.solve(b);

            if(solver.info() == Eigen::Success) {

                #ifdef PIC_DEBUG
                    printf("SOLVER SUCCESS!\n");
                #endif

                #pragma omp parallel for

                for(int j = 0; j < tot; j++) {
                    imgOut->data[j * imgOut->channels + i] = float(x(j));
                }
            } else {
                #ifdef PIC_DEBUG
                    printf("SOLVER FAILED!\n");
                #endif
            }

        }

        return imgOut;
    }

    float alpha, lambda, epsilon;

public:

    FilterWLS()
    {
        Update(1.2f, 1.0f);
    }

    FilterWLS(float alpha, float lambda)
    {
        Update(alpha, lambda);
    }

    void Update(float alpha, float lambda)
    {
        epsilon = 0.0001f;

        if(alpha <= 0.0f) {
            alpha = 1.2f;
        }

        if(lambda <= 0.0f) {
            lambda = 1.0f;
        }

        this->alpha = alpha;
        this->lambda = lambda;
    }

    ImageRAW *Process(ImageRAWVec imgIn, ImageRAW *imgOut)
    {
        if(imgIn.size() < 1){
            return imgOut;
        }

        if(imgIn[0] == NULL) {
            return imgOut;
        }

        imgOut = SetupAux(imgIn, imgOut);

        //Convolution
        if(imgIn[0]->channels == 1) {
            return SingleChannel(imgIn, imgOut);
        } else {
            return MultiChannel(imgIn, imgOut);
        }
    }

    ImageRAW *ProcessP(ImageRAWVec imgIn, ImageRAW *imgOut)
    {
        return Process(imgIn, imgOut);
    }

    static int main(int argc, char* argv[])
    {
        if(argc < 4) {
            printf("Usage: name_input alpha lambad\n");
            return 0;
        }

        std::string nameIn = argv[1];
        std::string name = removeExtension(nameIn);
        std::string ext = getExtension(nameIn);

        float alpha = float(atof(argv[2]));
        float lambda = float(atof(argv[3]));

        std::string nameOut = name + "_wls." + ext; 

        ImageRAW img(nameIn);

        FilterWLS *filter = new FilterWLS(alpha, lambda);

        filter->Process(Single(&img), NULL)->Write(nameOut);

        return 0;
    }
};

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_WLS_HPP */

#endif
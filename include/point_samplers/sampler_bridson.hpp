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

#ifndef PIC_POINT_SAMPLERS_SAMPLER_BRIDSON_HPP
#define PIC_POINT_SAMPLERS_SAMPLER_BRIDSON_HPP

#include <math.h>
#include <random>
#include "util/math.hpp"
#include "util/vec.hpp"

namespace pic {

/**
 *
 */
template<unsigned int N>
bool checkNeighborsBruteForce(std::vector< Vec<N, float> > &samples,
                              Vec<N, float> x, float radius)
{
    float radius2 = radius * radius;

    for(unsigned int i = 0; i < samples.size(); i++) {
        if(x.distanceSq(samples[i]) < radius2) {
            return false;
        }
    }

    return true;
}

/**
 *
 */
template<unsigned int N>
void BridsonSampler(std::mt19937 *m, float radius, std::vector<float> &samples,
                    int kSamples = 30)
{
    if(kSamples < 1) {
        kSamples = 30;
    }

    //Step 0: Creating an N-grid
//	Grid<N> grid(0.999f * radius / sqrtf(float(N)));

    //Step 1: Initial sample
    Vec<N, float> x0 = randomPoint<N>(m);

    std::vector< Vec<N, float> > vecSamples;
    std::vector<int> activeList;

    vecSamples.push_back(x0);
    activeList.push_back(0);
//	grid.setValue(0, x0);

    //Step 2: active list
    while(!activeList.empty()) {
        int i = (*m)() % activeList.size();

        int ind = activeList[i];

        bool bCheckSuccess = false;
        bool bFlag = true;

        int j = 0;

        while(bFlag) {
            //creating samples inside the annulus around sample_i
            Vec<N, float> x = annulusSampling<N>(m, vecSamples[ind], radius);

            //checking if the generated sample is in the bounding box
            if(insideVecBBox(x)) {
                //checking if sample does not have neighbors in grid with distance radius
                if(checkNeighborsBruteForce(vecSamples, x, radius)) {
                    vecSamples.push_back(x);
                    int value = vecSamples.size() - 1;

                    activeList.push_back(value);
                    //                grid.setValue(value, x);
                    bCheckSuccess = true;
                }
            }

            j++;

            bFlag = (j < kSamples) && (!bCheckSuccess);
        }

        if(!bCheckSuccess) { //removing i-th sample from the active list
            if(activeList.size() > 1) {
                activeList[i] = activeList.back();
                activeList.pop_back();
            } else {
                activeList.pop_back();
            }
        }
    }

    for(unsigned int i = 0; i < vecSamples.size(); i++) {
        Vec<N, float> x = vecSamples[i];

        for(unsigned int k = 0; k < N; k++) {
            samples.push_back(x[k]);
        }
    }
}

} // end namespace pic

#endif /* PIC_POINT_SAMPLERS_SAMPLER_BRIDSON_HPP */


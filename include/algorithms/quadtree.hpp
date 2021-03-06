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

#ifndef PIC_ALGORITHMS_QUADTREE_HPP
#define PIC_ALGORITHMS_QUADTREE_HPP

#include <set>

namespace pic {

class Quadtree
{
protected:
    bool						leaf;
    std::set<int>				list;
    Quadtree					*children[4];

    //Bounding box
    int							bmax[2], bmin[2];

    void FindAux(int *pos, int radius2, std::set<int> &out)
    {
        if(leaf) {
            out.insert(list.begin(), list.end());
        } else {
            for(int i = 0; i < 4; i++) {
                if(children[i] != NULL) {
                    if(CheckCircleBBox(children[i]->bmax, children[i]->bmin, pos, radius2)) {
                        children[i]->FindAux(pos, radius2, out);
                    }
                }
            }
        }
    }

public:

    Quadtree(int *bmax, int *bmin)
    {
        for(int i = 0; i < 2; i++) {
            this->bmax[i] = bmax[i];
            this->bmin[i] = bmin[i];
        }

        leaf = false;

        for(int i = 0; i < 4; i++) {
            children[i] = NULL;
        }
    }

    ~Quadtree()
    {
        for(int i = 0; i < 4; i++)
            if(children[i] != NULL) {
                delete children[i];
            }
    }

    static bool CheckPointBBox(int *p, int *bmin, int *bmax)
    {
        return((p[0] >= bmin[0])
               && (p[1] >= bmin[1])
               && (p[0] < bmax[0])
               && (p[1] < bmax[1]));
    }

    static bool CheckCircleBBox(int *bmax, int *bmin, int *center, int radius2)
    {
        int dmin = 0;

        for(int i = 0; i < 2; i++) {
            if(center[i] < bmin[i]) {
                int tmp = center[i] - bmin[i];
                dmin += tmp * tmp;
            } else {
                if(center[i] > bmax[i]) {
                    int tmp = center[i] - bmax[i];
                    dmin += tmp * tmp;
                }
            }
        }

        return (dmin <= radius2);
    }

    static void GetQuadrant(int *bmax, int *bmin, int *pMax, int *pMin, int i)
    {
        int half[2];

        for(int j = 0; j < 2; j++) {
            half[j] = (bmax[j] + bmin[j]);

            if((half[j] % 2) == 0) {
                half[j] = half[j] >> 1;
            } else {
                half[j] = (half[j] >> 1) + 1;
            }
        }

        switch(i) {
        case 0: {
            pMin[0] = bmin[0];
            pMin[1] = bmin[1];

            pMax[0] = half[0];
            pMax[1] = half[1];
        }
        break;

        case 1: {
            pMin[0] = half[0];
            pMin[1] = bmin[1];

            pMax[0] = bmax[0];
            pMax[1] = half[1];
        }
        break;

        case 2: {
            pMin[0] = bmin[0];
            pMin[1] = half[1];

            pMax[0] = half[0];
            pMax[1] = bmax[1];
        }
        break;

        case 3: {
            pMin[0] = half[0];
            pMin[1] = half[1];

            pMax[0] = bmax[0];
            pMax[1] = bmax[1];
        }
        break;
        }
    }

    void Insert(int *pos, int value, int MAX_OCTREE_LEVEL, int level = 0)
    {
        if(level == MAX_OCTREE_LEVEL) {
            list.insert(value);
            leaf = true;
        } else {
            int pMax[2], pMin[2];

            for(int i = 0; i < 4; i++) {
                GetQuadrant(bmax, bmin, pMax, pMin, i);

                if(CheckPointBBox(pos, pMin, pMax)) {
                    if(children[i] == NULL) {
                        children[i] = new Quadtree(pMax, pMin);
                    }

                    children[i]->Insert(pos, value, MAX_OCTREE_LEVEL, level + 1);
                    break;
                }
            }
        }
    }

    void Find(float x, float y, float radius, std::set<int> &out)
    {

        int pos[2];
        pos[0] = int(x);
        pos[1] = int(y);
        int radius2 = int(ceilf(radius * radius));

        if(CheckPointBBox(pos, bmin, bmax)) {
            FindAux(pos, radius2, out);
        }
    }
};

} // end namespace pic

#endif /* PIC_ALGORITHMS_QUADTREE_HPP */


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

#ifndef PIC_GL_FILTERING_FILTER_1D_HPP
#define PIC_GL_FILTERING_FILTER_1D_HPP

#include "gl/filtering/filter.hpp"

namespace pic {

/**
 * @brief The FilterGL1D class
 */
class FilterGL1D: public FilterGL
{
protected:
    ImageRAWGL	*weights;

    int			dirs[3];
    int			slice;

    /**
     * @brief InitShaders
     */
    void InitShaders();

    /**
     * @brief FragmentShader
     */
    virtual void FragmentShader()
    {

    }

public:

    /**
     * @brief FilterGL1D
     * @param direction
     * @param target
     */
    FilterGL1D(int direction, GLenum target);

    /**
     * @brief ChangePass
     * @param pass
     * @param tPass
     */
    void ChangePass(int pass, int tPass);

    /**
     * @brief SetUniformAux
     */
    virtual void SetUniformAux()
    {

    }

    /**
     * @brief SetUniform
     */
    void SetUniform();

    /**
     * @brief setSlice
     * @param slice
     */
    void setSlice(int slice)
    {
        this->slice = slice;
        SetUniform();
    }

    /**
     * @brief setSlice2
     * @param slice
     */
    void setSlice2(int slice)
    {
        this->slice = slice;
        filteringProgram.uniform("slice", slice);
    }

    /**
     * @brief Process
     * @param imgIn
     * @param imgOut
     * @return
     */
    ImageRAWGL *Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut);
};

FilterGL1D::FilterGL1D(int direction, GLenum target): FilterGL()
{
    weights = NULL;

    //protected values are assigned/computed
    this->target = target;

    slice = 0;

    dirs[0] = dirs[1] = dirs[2] = 0;

    switch(target) {
    case GL_TEXTURE_2D:
        dirs[direction % 2] = 1;
        break;

    case GL_TEXTURE_2D_ARRAY:
        dirs[direction % 3] = 1;
        break;

    case GL_TEXTURE_3D:
        dirs[direction % 3] = 1;
        break;
    }

}

void FilterGL1D::ChangePass(int pass, int tPass)
{

    if(target == GL_TEXTURE_2D) {
        dirs[  pass % 2 ] = 1;
        dirs[(pass + 1) % 2 ] = 0;
    }

    if(target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY) {
        dirs[  pass % 3 ] = 1;
        dirs[(pass + 1) % 3 ] = 0;
        dirs[(pass + 2) % 3 ] = 0;
    }

#ifdef PIC_DEBUG
//    printf("%d %d %d\n", dirs[0], dirs[1], dirs[2]);
#endif

    SetUniform();
}

void FilterGL1D::SetUniform()
{
    glw::bind_program(filteringProgram);
    filteringProgram.uniform("u_tex", 0);
    filteringProgram.uniform("iX", dirs[0]);
    filteringProgram.uniform("iY", dirs[1]);

    if(target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY) {
        filteringProgram.uniform("iZ", dirs[2]);
        filteringProgram.uniform("slice", slice);
    }

    SetUniformAux();

    glw::bind_program(0);
}

void FilterGL1D::InitShaders()
{
    filteringProgram.setup(glw::version("330"), vertex_source, fragment_source);

#ifdef PIC_DEBUG
    printf("[FilterGL1D shader log]\n%s\n", filteringProgram.log().c_str());
#endif

    glw::bind_program(filteringProgram);
    filteringProgram.attribute_source("a_position", 0);
    filteringProgram.fragment_target("f_color", 0);
    filteringProgram.relink();

    SetUniform();
}

ImageRAWGL *FilterGL1D::Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut)
{
    if(imgIn[0] == NULL || imgIn.size() > 1) {
        return imgOut;
    }

    int w = imgIn[0]->width;
    int h = imgIn[0]->height;
    int f = imgIn[0]->frames;

    if(imgOut == NULL) {
        imgOut = new ImageRAWGL(f, w, h, 4, IMG_GPU, imgIn[0]->getTarget());
    }

    if(fbo == NULL) {
        fbo = new Fbo();
    }

    fbo->create(w, h, f, false, imgOut->getTexture());

    ImageRAWGL *base = imgIn[0];

    //Textures
    glActiveTexture(GL_TEXTURE0);
    base->bindTexture();

    if(weights != NULL) {
        glActiveTexture(GL_TEXTURE1);
        weights->bindTexture();
    }

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    //Shaders
    glw::bind_program(filteringProgram);

    //Rendering
    fbo->bind2();

    //Rendering aligned quad
    for(int z = 0; z < f; z++) {
        setSlice2(z);
        fbo->attachColorBuffer2(0, target, z);

        quad->Render();
    }

    //Fbo
    fbo->unbind2();

    //Shaders
    glw::bind_program(0);

    //Textures
    if(weights != NULL) {
        glActiveTexture(GL_TEXTURE1);
        weights->unBindTexture();
    }

    glActiveTexture(GL_TEXTURE0);
    base->unBindTexture();

    return imgOut;
}

} // end namespace pic

#endif /* PIC_GL_FILTERING_FILTER_1D_HPP */


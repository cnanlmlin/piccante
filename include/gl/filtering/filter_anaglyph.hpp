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

#ifndef PIC_GL_FILTERING_FILTER_ANAGLYPH_HPP
#define PIC_GL_FILTERING_FILTER_ANAGLYPH_HPP

#include "gl/filtering/filter.hpp"

namespace pic {

/**
 * @brief The FilterGLAnaglyph class
 */
class FilterGLAnaglyph: public FilterGL
{
protected:

    /**
     * @brief InitShaders
     */
    void InitShaders();

public:
    /**
     * @brief FilterGLAnaglyph
     */
    FilterGLAnaglyph();

    /**
     * @brief Process
     * @param imgIn
     * @param imgOut
     * @return
     */
    ImageRAWGL *Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut);
};

FilterGLAnaglyph::FilterGLAnaglyph(): FilterGL()
{
    InitShaders();
}

void FilterGLAnaglyph::InitShaders()
{
    fragment_source = GLW_STRINGFY
                      (
                          uniform sampler2D u_texL; \n
                          uniform sampler2D u_texR; \n
                          out     vec4      f_color; \n

    void main(void) {
        \n
        ivec2 coords = ivec2(gl_FragCoord.xy);
        \n
        vec3  colL = texelFetch(u_texL, coords, 0).xyz;
        \n
        vec3  colR = texelFetch(u_texR, coords, 0).xyz;
        \n
        vec3  colA = vec3(colL.x, colR.y, colR.z);
        colA = pow(colA, vec3(0.45));
        f_color = vec4(colA, 1.0);
        \n
    }
                      );


    filteringProgram.setup(glw::version("330"), vertex_source, fragment_source);

#ifdef PIC_DEBUG
    printf("[filteringProgram log]\n%s\n", filteringProgram.log().c_str());
#endif

    glw::bind_program(filteringProgram);
    filteringProgram.attribute_source("a_position", 0);
    filteringProgram.fragment_target("f_color",    0);
    filteringProgram.relink();
    filteringProgram.uniform("u_texL",      0);
    filteringProgram.uniform("u_texR",      1);
    glw::bind_program(0);
}

//Processing
ImageRAWGL *FilterGLAnaglyph::Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut)
{
    if((imgIn[0] == NULL) && (imgIn[0]->channels != 3)) {
        return imgOut;
    }

    int w = imgIn[0]->width;
    int h = imgIn[0]->height;

    if(imgOut == NULL) {
        imgOut = new ImageRAWGL(1, w, h, imgIn[0]->channels, IMG_GPU, GL_TEXTURE_2D);
    }

    if(fbo == NULL) {
        fbo = new Fbo();
    }

    fbo->create(w, h, 1, false, imgOut->getTexture());

    //Rendering
    fbo->bind();
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    //Shaders
    glw::bind_program(filteringProgram);

    //Textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, imgIn[1]->getTexture());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imgIn[0]->getTexture());

    //Rendering aligned quad
    quad->Render();

    //Fbo
    fbo->unbind();

    //Shaders
    glw::bind_program(0);

    //Textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return imgOut;
}

} // end namespace pic

#endif /* PIC_GL_FILTERING_FILTER_ANAGLYPH_HPP */


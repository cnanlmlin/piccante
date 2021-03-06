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

#ifndef PIC_GL_FILTERING_FILTER_DRAGO_TMO_HPP
#define PIC_GL_FILTERING_FILTER_DRAGO_TMO_HPP

#include "gl/filtering/filter.hpp"
#include "gl/filtering/filter_luminance.hpp"

namespace pic {

/**
 * @brief The FilterGLDragoTMO class
 */
class FilterGLDragoTMO: public FilterGL
{
protected:
    float b, Ld_Max, LMax, LMax_scaled, Lwa, Lwa_scaled;
    float constant1, constant2;
    bool bGammaCorrection;

    /**
     * @brief ComputeConstants
     */
    void ComputeConstants();

    /**
     * @brief InitShaders
     */
    void InitShaders();

    /**
     * @brief FragmentShader
     */
    void FragmentShader();

public:
    /**
     * @brief FilterGLDragoTMO
     */
    FilterGLDragoTMO();

    /**
     * @brief FilterGLDragoTMO
     * @param Ld_Max
     * @param b
     * @param LMax
     * @param Lwa
     * @param bGammaCorrection
     */
    FilterGLDragoTMO(float Ld_Max, float b, float LMax, float Lwa,
                     bool bGammaCorrection);

    void Update(float Ld_Max, float b, float LMax);

    /**
     * @brief Process
     * @param imgIn
     * @param imgOut
     * @return
     */
    ImageRAWGL *Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut);

};

FilterGLDragoTMO::FilterGLDragoTMO(): FilterGL()
{
    Ld_Max	=  100.0f;
    b		=  0.95f;
    LMax	=  1e6f;
    Lwa		= -1.0f;

    bGammaCorrection = false;

    FragmentShader();
    InitShaders();
}

FilterGLDragoTMO::FilterGLDragoTMO(float Ld_Max, float b, float LMax, float Lwa,
                                   bool bGammaCorrection = false): FilterGL()
{
    //protected values are assigned/computed
    if(Ld_Max > 0.0f) {
        this->Ld_Max = Ld_Max;
    } else {
        this->Ld_Max = 100.0f;
    }

    if(b > 0.0f) {
        this->b = b;
    } else {
        this->b = 0.95f;
    }

    if(LMax > 0.0f) {
        this->LMax = LMax;
    } else {
        this->LMax = 1e6f;
    }

    if(Lwa > 0.0f) {
        this->Lwa = Lwa;
    } else {
        this->Lwa = 1.0f;
    }

    this->bGammaCorrection = bGammaCorrection;

    FragmentShader();
    InitShaders();
}

void FilterGLDragoTMO::FragmentShader()
{
    fragment_source = GLW_STRINGFY
                      (
    uniform sampler2D u_tex;	\n
    uniform sampler2D u_lum;	\n
    uniform float	  constant1;\n
    uniform float	  constant2;\n
    uniform float     LMax;		\n
    uniform float     Lwa;		\n
    out     vec4      f_color;	\n

    void main(void) {
        \n
        ivec2 coords   = ivec2(gl_FragCoord.xy);\n
        vec3  color    = texelFetch(u_tex, coords, 0).xyz;\n
        float L        = texelFetch(u_lum, coords, 0).x;\n
        float L_scaled = L / Lwa;\n
        float tmp      = pow((L_scaled / LMax), constant1);\n
        float Ld       = constant2 * log(1.0 + L_scaled) / log(2.0 + 8.0 * tmp);\n
        color		   = (color * Ld) / L;\n
        __GAMMA__CORRECTION__ \n
        f_color        = vec4(color, 1.0);\n
        \n
    }\n
                      );

    fragment_source = GammaCorrection(fragment_source, bGammaCorrection);
}

void FilterGLDragoTMO::ComputeConstants()
{
    Lwa_scaled  = Lwa / powf(1.0f + b - 0.85f, 5.0f);
    LMax_scaled = LMax / Lwa_scaled;
    constant1   = logf(b) / logf(0.5f);
    constant2   = (Ld_Max / 100.0f) / (log10(1 + LMax_scaled));
}

void FilterGLDragoTMO::InitShaders()
{
    filteringProgram.setup(glw::version("330"), vertex_source, fragment_source);

#ifdef PIC_DEBUG
    printf("[FilterGLDragoTMO log]\n%s\n", filteringProgram.log().c_str());
#endif

    glw::bind_program(filteringProgram);
    filteringProgram.attribute_source("a_position", 0);
    filteringProgram.fragment_target("f_color",    0);
    filteringProgram.relink();

    Update(Ld_Max, b, LMax);
}

void FilterGLDragoTMO::Update(float Ld_Max, float b, float LMax)
{
    if(Ld_Max > 0.0f) {
        this->Ld_Max = Ld_Max;
    } else {
        this->Ld_Max = 100.0f;
    }

    if(b > 0.0f) {
        this->b = b;
    } else {
        this->b = 0.95f;
    }

    if(LMax > 0.0f) {
        this->LMax = LMax;
    } else {
        this->LMax = 1e6f;
    }

    ComputeConstants();

    glw::bind_program(filteringProgram);
    filteringProgram.uniform("u_tex",       0);
    filteringProgram.uniform("u_lum",       1);
    filteringProgram.uniform("constant1",	constant1);
    filteringProgram.uniform("constant2",	constant2);
    filteringProgram.uniform("LMax",	    LMax_scaled);
    filteringProgram.uniform("Lwa",		    Lwa_scaled);
    glw::bind_program(0);   
}

ImageRAWGL *FilterGLDragoTMO::Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut)
{
    if(imgIn.size()<2) {
        return imgOut;
    }

    if(imgIn[0] == NULL) {
        return imgOut;
    }

    int w = imgIn[0]->width;
    int h = imgIn[0]->height;

    if(imgOut == NULL) {
        imgOut = new ImageRAWGL(1, w, h, imgIn[0]->channels, IMG_GPU, GL_TEXTURE_2D);
    }

    //Fbo
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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return imgOut;
}

} // end namespace pic

#endif /* PIC_GL_FILTERING_FILTER_DRAGO_TMO_HPP */


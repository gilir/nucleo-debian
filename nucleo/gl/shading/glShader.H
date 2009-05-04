#ifndef n_glShader_H
#define n_glShader_H

/*
 *
 * nucleo/gl/shading/glShader.H --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/gl/glIncludes.H>

#include <map>

namespace nucleo {

  class glShader {

  protected:

#if defined(GL_ARB_shader_objects)
    GLhandleARB program ;
    std::map<std::string, GLhandleARB> shaders ;
#endif

  public:

    glShader(void) ;
    ~glShader(void) ;

    // Attach a GLSL shader. Type can be "vertex" or "fragment"
    bool attach(std::string name, std::string type, const char *text) ;
    bool attachFromFile(std::string name, std::string type, std::string filename) ;

    bool link(void) ;

    void activate(void) ;
    bool isActive(void) ;
    void deactivateAllShaders(void) ;

  private:

    bool _prepareSetUnifomaPara(std::string param_name, GLint *param);

  public:

    bool setUniformParam(std::string param_name, GLint v);
    bool setUniformParam(std::string param_name, GLint v1, GLint v2);
    bool setUniformParam(std::string param_name, GLint v1, GLint v2, GLint v3);
    bool setUniformParam(std::string param_name, GLint v1, GLint v2, GLint v3, GLint v4);
    bool setUniformParam(std::string param_name, GLsizei count, GLint *v);
    bool setUniformParam(std::string param_name, GLfloat v);
    bool setUniformParam(std::string param_name, GLfloat v1, GLfloat v2);
    bool setUniformParam(std::string param_name, GLfloat v1, GLfloat v2, GLfloat v3);
    bool setUniformParam(std::string param_name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);
    bool setUniformParam(std::string param_name, GLsizei count, GLfloat *v);
  } ;

}

#endif

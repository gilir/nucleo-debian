/*
 *
 * nucleo/gl/shading/glShader.cxx --
 *
 * Copyright (C) Nicolas Roussel
 *
 * See the file LICENSE for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <nucleo/config.H>
#include <nucleo/nucleo.H>
#include <nucleo/gl/shading/glShader.H>
#include <nucleo/utils/FileUtils.H>

#include <iostream>

namespace nucleo {

  typedef enum {UNKNOWN, SUPPORTED, UNSUPPORTED} glslSupport ;

#if HAVE_AGL
  static glslSupport glsl_support = SUPPORTED ;

#elif HAVE_GLX

#include <GL/glx.h>
#include <GL/glxext.h>

#if defined(GL_ARB_shader_objects)
  static glslSupport glsl_support = UNKNOWN ;
  
#if defined(HAVE_glXGetProcAddress)
  #warning Using glXGetProcAddress

#define ARB_DECLARATION(x,y) x##ARBPROC y = 0
#define NOARB_DECLARATION(x,y) x##PROC y = 0
#define GLX_SETPROCADDRESS(n,m,x) n##ARB = (m##ARBPROC)glXGetProcAddress(x)
#define NOARB_GLX_SETPROCADDRESS(n,m,x) n##ARB = (m##PROC)glXGetProcAddress(x)

#elif defined(HAVE_glXGetProcAddressARB)
  #warning Using glXGetProcAddressARB

#define ARB_DECLARATION(x,y) x##ARBPROC y = 0
#define NOARB_DECLARATION(x,y) x##PROC y = 0
#define GLX_SETPROCADDRESS(n,m,x) n##ARB = (m##ARBPROC)glXGetProcAddressARB(x)
#define NOARB_GLX_SETPROCADDRESS(n,m,x) n##ARB = (m##PROC)glXGetProcAddressARB(x)

#else
  #error GL_ARB_shader_objects defined and no glXGetProcAddress* functions

#endif // HAVE_glXGetProcAddress*

#else // GL_ARB_shader_objects
  #warning GLSL shaders will be disabled: no glXGetProcAddress* function

static glslSupport glsl_support =  UNSUPPORTED;

#endif // GL_ARB_shader_objects

#if defined(GL_ARB_shader_objects)
  ARB_DECLARATION(PFNGLCREATEPROGRAMOBJECT,     glCreateProgramObjectARB);
  ARB_DECLARATION(PFNGLCREATESHADEROBJECT,      glCreateShaderObjectARB);
  ARB_DECLARATION(PFNGLSHADERSOURCE,            glShaderSourceARB);
  ARB_DECLARATION(PFNGLCOMPILESHADER,           glCompileShaderARB);
  ARB_DECLARATION(PFNGLGETOBJECTPARAMETERIV,    glGetObjectParameterivARB);
  ARB_DECLARATION(PFNGLATTACHOBJECT,            glAttachObjectARB);
  ARB_DECLARATION(PFNGLGETINFOLOG,              glGetInfoLogARB);
  ARB_DECLARATION(PFNGLLINKPROGRAM,             glLinkProgramARB);
  ARB_DECLARATION(PFNGLUSEPROGRAMOBJECT,        glUseProgramObjectARB);
  ARB_DECLARATION(PFNGLGETUNIFORMLOCATION,      glGetUniformLocationARB);
  ARB_DECLARATION(PFNGLGETHANDLE,               glGetHandleARB);
  //
  NOARB_DECLARATION(PFNGLUNIFORM1I,               glUniform1iARB);
  NOARB_DECLARATION(PFNGLUNIFORM2I,               glUniform2iARB);
  NOARB_DECLARATION(PFNGLUNIFORM3I,               glUniform3iARB);
  NOARB_DECLARATION(PFNGLUNIFORM4I,               glUniform4iARB);
  NOARB_DECLARATION(PFNGLUNIFORM1IV,              glUniform1ivARB);
  NOARB_DECLARATION(PFNGLUNIFORM1F,               glUniform1fARB);
  NOARB_DECLARATION(PFNGLUNIFORM2F,               glUniform2fARB);
  NOARB_DECLARATION(PFNGLUNIFORM3F,               glUniform3fARB);
  NOARB_DECLARATION(PFNGLUNIFORM4F,               glUniform4fARB);
  NOARB_DECLARATION(PFNGLUNIFORM1FV,              glUniform1fvARB);
#endif

  static void
  findGLSLprocs(void) {
#if defined(GL_ARB_shader_objects)
    GLX_SETPROCADDRESS(glCreateProgramObject, PFNGLCREATEPROGRAMOBJECT,
     (const GLubyte *)"glCreateProgramObjectARB");
    GLX_SETPROCADDRESS(glCreateShaderObject, PFNGLCREATESHADEROBJECT,
     (const GLubyte *)"glCreateShaderObjectARB");
    GLX_SETPROCADDRESS(glShaderSource, PFNGLSHADERSOURCE,
     (const GLubyte *)"glShaderSourceARB");
    GLX_SETPROCADDRESS(glCompileShader, PFNGLCOMPILESHADER,
     (const GLubyte *)"glCompileShaderARB");
    GLX_SETPROCADDRESS(glGetObjectParameteriv, PFNGLGETOBJECTPARAMETERIV,
     (const GLubyte *)"glGetObjectParameterivARB");
    GLX_SETPROCADDRESS(glGetInfoLog, PFNGLGETINFOLOG,
     (const GLubyte *)"glGetInfoLogARB");
    GLX_SETPROCADDRESS(glAttachObject, PFNGLATTACHOBJECT,
     (const GLubyte *)"glAttachObjectARB");
    GLX_SETPROCADDRESS(glLinkProgram, PFNGLLINKPROGRAM,
     (const GLubyte *)"glLinkProgramARB");
    GLX_SETPROCADDRESS(glGetUniformLocation, PFNGLGETUNIFORMLOCATION,
     (const GLubyte *)"glGetUniformLocationARB");
    GLX_SETPROCADDRESS(glUseProgramObject, PFNGLUSEPROGRAMOBJECT,
     (const GLubyte *)"glUseProgramObjectARB");
    GLX_SETPROCADDRESS(glGetHandle, PFNGLGETHANDLE,
     (const GLubyte *)"glGetHandleARB");
    //
    NOARB_GLX_SETPROCADDRESS(glUniform1i, PFNGLUNIFORM1I,
	   (const GLubyte *)"glUniform1iARB");
    NOARB_GLX_SETPROCADDRESS(glUniform2i, PFNGLUNIFORM2I,
	   (const GLubyte *)"glUniform2iARB");
    NOARB_GLX_SETPROCADDRESS(glUniform3i, PFNGLUNIFORM3I,
	   (const GLubyte *)"glUniform3iARB");
    NOARB_GLX_SETPROCADDRESS(glUniform4i, PFNGLUNIFORM4I,
	   (const GLubyte *)"glUniform4iARB");
    NOARB_GLX_SETPROCADDRESS(glUniform1iv, PFNGLUNIFORM1IV,
	   (const GLubyte *)"glUniform1ivARB" );
    NOARB_GLX_SETPROCADDRESS(glUniform1f, PFNGLUNIFORM1F,
	   (const GLubyte *)"glUniform1fARB");
    NOARB_GLX_SETPROCADDRESS(glUniform2f, PFNGLUNIFORM2F,
	   (const GLubyte *)"glUniform2fARB");
    NOARB_GLX_SETPROCADDRESS(glUniform3f, PFNGLUNIFORM3F,
	   (const GLubyte *)"glUniform3fARB");
    NOARB_GLX_SETPROCADDRESS(glUniform4f, PFNGLUNIFORM4F,
	   (const GLubyte *)"glUniform4fARB");
    NOARB_GLX_SETPROCADDRESS(glUniform1fv, PFNGLUNIFORM1FV,
	   (const GLubyte *)"glUniform1fvARB");

    if (glCreateProgramObjectARB !=0 
	&& glCreateShaderObjectARB !=0
	&& glShaderSourceARB !=0
	&& glCompileShaderARB !=0
	&& glGetObjectParameterivARB !=0
	&& glGetInfoLogARB !=0
	&& glAttachObjectARB !=0
	&& glLinkProgramARB !=0
	&& glGetUniformLocationARB !=0
	&& glUseProgramObjectARB !=0
	&& glGetHandleARB !=0
	&& glUniform1iARB !=0
	&& glUniform2iARB !=0
	&& glUniform3iARB !=0
	&& glUniform4iARB !=0
	&& glUniform1ivARB !=0
	&& glUniform1fARB !=0
	&& glUniform2fARB !=0
	&& glUniform3fARB !=0
	&& glUniform4fARB !=0
	&& glUniform1fvARB !=0)
    {
	    glsl_support = SUPPORTED ;
    }
    else
    {
	    std::cerr << "glShader: GLX_SETPROCADDRESS set at least One func to zero "
		      << glCreateProgramObjectARB << " " 
		      << glCreateShaderObjectARB << " "
		      << glShaderSourceARB << " "
		      << glCompileShaderARB << " "
		      << glGetObjectParameterivARB << " "
		      << glGetInfoLogARB << " "
		      << glAttachObjectARB << " "
		      << glLinkProgramARB << " "
		      << glGetUniformLocationARB << " "
		      << glUseProgramObjectARB << " "
		      << glGetHandleARB << " "
		      << glUniform1iARB << " "
		      << glUniform2iARB << " "
		      << glUniform3iARB << " "
		      << glUniform4iARB << " "
		      << glUniform1ivARB << " "
		      << glUniform1fARB << " "
		      << glUniform2fARB << " "
		      << glUniform3fARB << " "
		      << glUniform4fARB << " "
		      << glUniform1fvARB << "\n";

	    glsl_support = UNSUPPORTED ;
    }
#else // GL_ARB_shader_objects
    glsl_support = UNSUPPORTED ;
#endif // GL_ARB_shader_objects

    std::cerr << "glShader: GLSL shaders seem to be " << (glsl_support==SUPPORTED?"":"un") << "supported" << std::endl ;
  }

#undef ARB_DECLARATION
#undef NOARB_DECLARATION
#undef GLX_SETPROCADDRESS
#undef NOARB_GLX_SETPROCADDRESS

#endif // HAVE_GLX

  // -----------------------------------------------------------------------

  glShader::glShader(void) {
#if defined(GL_ARB_shader_objects)
#if HAVE_GLX
    if (glsl_support==UNKNOWN) findGLSLprocs() ;
#endif
    if (glsl_support==SUPPORTED) {
	 program = glCreateProgramObjectARB() ;
    } else {
	 program = 0 ;
    }
#endif
  }

  glShader::~glShader(void) {
#if defined(GL_ARB_shader_objects)
    // FIXME: release shaders and program
#endif
  }

  bool
  glShader::attach(std::string name, std::string type, const char *text) {
#if defined(GL_ARB_shader_objects)
    if (glsl_support!=SUPPORTED) return false ;

    GLhandleARB s = 0 ;
    if (type=="vertex") 
	 s = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB) ;
    else if (type=="fragment") 
	 s = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB) ;
    else {
	 std::cerr << "glShader::attach (" << name << "): unknown shader type '" << type << "'" << std::endl ;
	 return false ;
    }

    glShaderSourceARB(s, 1, (const GLcharARB**)&text, NULL);
    glCompileShaderARB(s) ;

    GLint logLength ;
    glGetObjectParameterivARB(s, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength) ;
    if (logLength > 0) {
	 GLcharARB *log = new GLcharARB [logLength] ;
	 glGetInfoLogARB(s, logLength, &logLength, log) ;
	 std::cerr << "glShader::attach (" << this << ", " << name << "): " << logLength << " bytes" << std::endl ;
	 if (logLength) std::cerr << log << std::endl ;
	 delete [] log ;
    }

    GLint compstatus;
    glGetObjectParameterivARB(s, GL_OBJECT_COMPILE_STATUS_ARB, &compstatus) ;
    if (compstatus == 0)
	 std::cerr << "glShader::attach (" << this << ", " << name << "): compilation failed" << std::endl ;
    else
	 glAttachObjectARB(program, s) ;
    shaders[name] = s ;
    return true ;
#else
    return false ;
#endif
  }

  bool
  glShader::attachFromFile(std::string name, std::string type, std::string filename) {
#if defined(GL_ARB_shader_objects)
    if (glsl_support!=SUPPORTED) return false ;

    uint64_t size = getFileSize(filename.c_str()) ;
    if (!size) {
	 filename = getNucleoResourcesDirectory()+filename ;
	 size = getFileSize(filename.c_str()) ;
    }
    if (!size) {
	 std::cerr << "glShader::attachFromFile (" << this << "): unable to find " << filename << std::endl ;
	 return false ;
    }
    unsigned char *text = new unsigned char [size+1] ;
    text[size] = '\0' ;
    readFromFile(filename.c_str(), text, size) ;
    bool result = attach(name, type, (const GLcharARB*)text) ;
    delete [] text ;
    return result ;
#else
    return false ;
#endif
  }

  bool
  glShader::link(void) {
#if defined(GL_ARB_shader_objects)
    if (glsl_support!=SUPPORTED) return false ;

    glLinkProgramARB(program) ;

    GLint logLength ;
    glGetObjectParameterivARB(program, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength) ;
    if (logLength > 0) {
	 GLcharARB *log = new GLcharARB [logLength] ;
	 glGetInfoLogARB(program, logLength, &logLength, log) ;
	 std::cerr << "glShader::link log (" << this << "): " << logLength << " bytes" << std::endl ;
	 if (logLength) std::cerr << log << std::endl ;
	 delete [] log ;
    }
	
    GLint compstatus ;
    glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &compstatus);
    if (compstatus == 0) {
	 std::cerr << "glShader::link (" << this << "): link failed" << std::endl ;
	 return false ;
    }

    return true ;
#else
    return false ;
#endif
  }

  void
  glShader::activate(void) {
#if defined(GL_ARB_shader_objects)
    if (glsl_support==SUPPORTED)
	 glUseProgramObjectARB(program) ;
#endif
  }

  bool
  glShader::isActive(void) {
#if defined(GL_ARB_shader_objects)
    if (glsl_support!=SUPPORTED) return false ;
    return (program==glGetHandleARB(GL_PROGRAM_OBJECT_ARB)) ;
#else
    return false ;
#endif
  }
  
  void
  glShader::deactivateAllShaders(void) {
#if defined(GL_ARB_shader_objects)
    if (glsl_support==SUPPORTED)
	 glUseProgramObjectARB(0) ;
#endif
  }

  // -----------------------------------------------------------------------


  bool glShader::_prepareSetUnifomaPara(std::string param_name, GLint *param)
  {
	  bool rc = false;
#if defined(GL_ARB_shader_objects)
	  if (!isActive()) {
		  activate();
		  rc = true;
	  }
	  *param = glGetUniformLocationARB(program, (const GLcharARB*)param_name.c_str());
#endif
	  return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLint v)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform1iARB(param, v);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLint v1, GLint v2)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform2iARB(param, v1, v2);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLint v1, GLint v2, GLint v3)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform3iARB(param, v1, v2, v3);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLint v1, GLint v2, GLint v3, GLint v4)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform4iARB(param, v1, v2, v3, v4);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLsizei count, GLint *v)
  {
	 bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform1ivARB(param, count, v);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc; 
  }

  bool
  glShader::setUniformParam(std::string param_name, GLfloat v)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform1fARB(param, v);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLfloat v1, GLfloat v2)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform2fARB(param, v1, v2);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLfloat v1, GLfloat v2, GLfloat v3)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform3fARB(param, v1, v2, v3);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4)
  {
	bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform4fARB(param, v1, v2, v3, v4);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc;
  }

  bool
  glShader::setUniformParam(std::string param_name, GLsizei count, GLfloat *v)
  {
	 bool rc = false;
#if defined(GL_ARB_shader_objects)
	if (glsl_support!=SUPPORTED) return false;

	GLint param;
	bool do_deactivate = _prepareSetUnifomaPara(param_name, &param);
	if (param!=-1) {
		glUniform1fvARB(param, count, v);
		rc = true;
	}
	if (do_deactivate)
		deactivateAllShaders();
#endif
	return rc; 
  }

}


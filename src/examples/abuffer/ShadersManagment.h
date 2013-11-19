/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#ifndef SHADERSMANAGMENT_H
#define SHADERSMANAGMENT_H

//Windows header
#ifdef WIN32
#include <windows.h>
#endif

#include <jagDraw/PlatformOpenGL.h>

#include <jagDraw/Error.h>

#include <string>

///////////////////////////////////////////
std::string loadTextFile(const char *name);
GLuint createShader(const char *fileName, GLuint shaderType, GLuint shaderID=0);
void checkProgramInfos(GLuint programID, GLuint stat);
GLuint createShaderProgram(const char *fileNameVS, const char *fileNameGS, const char *fileNameFS, GLuint programID=0);
void linkShaderProgram(GLuint programID);

void setShadersGlobalMacro(const char *macro, int val);
void setShadersGlobalMacro(const char *macro, float val);
void resetShadersGlobalMacros();
///////////////////////////////////////////



inline void checkGLError(const char *functionName) {
    jagDraw::errorCheck( std::string( functionName ) );
    /*
   GLenum error;
   while (( error = glGetError() ) != GL_NO_ERROR) {
      fprintf (stderr, "[%s] GL error %s \n", functionName, gluErrorString(error));
   }
   */
}


#endif
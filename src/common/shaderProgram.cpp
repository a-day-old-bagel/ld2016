/*
 * Copyright (c) 2016 Jonathan Glines
 * Jonathan Glines <jonathan@glines.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cstring>

#include "shaderProgram.h"

namespace ld2016 {
  ShaderProgram::ShaderProgram(
      const std::string &vert, const std::string &frag)
  {
    char *vertCode, *fragCode;
    unsigned int vertCodeLen, fragCodeLen;

    // Read the vertex and fragment shaders from file
    m_readCode(vert, &vertCode, &vertCodeLen);
    m_readCode(frag, &fragCode, &fragCodeLen);

    // Compile and link our shader
    m_init(
        vertCode, vertCodeLen,
        fragCode, fragCodeLen);

    delete[] vertCode;
    delete[] fragCode;
  }

  ShaderProgram::ShaderProgram(
      const char *vert, unsigned int vert_len,
      const char *frag, unsigned int frag_len)
  {
    // Compile and link our shader
    m_init(vert, vert_len, frag, frag_len);
  }

  ShaderProgram::~ShaderProgram() {
  }

  void ShaderProgram::m_init(
      const char *vert, unsigned int vert_len,
      const char *frag, unsigned int frag_len)
  {
    m_shaderProgram = glCreateProgram();

    GLuint vertexShader = -1;
    if (vert_len != 0) {
      vertexShader = m_compileShader(vert, vert_len, GL_VERTEX_SHADER);
      glAttachShader(m_shaderProgram, vertexShader);
    }
    GLuint fragmentShader = -1;
    if (frag_len != 0) {
      fragmentShader = m_compileShader(frag, frag_len, GL_FRAGMENT_SHADER);
      glAttachShader(m_shaderProgram, fragmentShader);
    }

    m_linkShaderProgram(m_shaderProgram);

    initLocations();
  }

  void ShaderProgram::m_readCode(
      const std::string &path, char **code, unsigned int *code_len)
  {
    FILE *f;
    unsigned int length;

    f = fopen(path.c_str(), "r");
    if (f == NULL) {
      fprintf(stderr, "Could not open shader file: %s\n", path.c_str());
      exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_END);
    *code_len = ftell(f);
    rewind(f);
    *code = new char[*code_len];
    length = fread(*code, 1, *code_len, f);
    if (length != *code_len) {
      fprintf(stderr, "Could not read shader file: %s\n", path.c_str());
      exit(EXIT_FAILURE);
    }
  }

  void ShaderProgram::use() const {
    glUseProgram(m_shaderProgram);
  }

  GLuint ShaderProgram::m_compileShader(
      const char *code, int code_len, GLenum type)
  {
    std::stringstream ss;
#ifndef __EMSCRIPTEN__
    ss << "#version 100\n";
    code_len += strlen("#version 100\n");
#endif
    ss << code;

    GLint status;

    GLuint shader = glCreateShader(type);

    std::string temp = ss.str();
    const char* finalCode = temp.c_str();

    glShaderSource(shader, 1, &finalCode, &code_len);

    glCompileShader(shader);
    glGetShaderiv(
        shader,
        GL_COMPILE_STATUS,
        &status);
    if (status != GL_TRUE) {
      char *log;
      int logLength;
      glGetShaderiv(
          shader,
          GL_INFO_LOG_LENGTH,
          &logLength);
      log = (char*)malloc(logLength);
      glGetShaderInfoLog(
          shader,
          logLength,
          NULL,
          log);
      // FIXME: The shader path should be printed with this error
      fprintf(stderr, "Error compiling shader %s: %s\n",
        finalCode, log);
      free(log);
      // FIXME: Maybe call something other than exit() here
      exit(EXIT_FAILURE);
    }

    return shader;
  }

  void ShaderProgram::m_linkShaderProgram(GLuint shaderProgram) {
    GLint status;

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
      char *log;
      int logLength;
      glGetProgramiv(
          shaderProgram,
          GL_INFO_LOG_LENGTH,
          &logLength);
      log = (char *)malloc(logLength);
      glGetProgramInfoLog(
          shaderProgram,
          logLength,
          NULL,
          log);
      fprintf(stderr, "Error linking shader program: %s\n", log);
      free(log);
      // FIXME: Maybe call something other than exit() here
      exit(EXIT_FAILURE);
    }
  }

  void ShaderProgram::initLocations() {
    // Register some common uniform and attribute locations
    m_modelViewLocation = glGetUniformLocation(
        m_shaderProgram, "modelView");
    m_projectionLocation = glGetUniformLocation(
        m_shaderProgram, "projection");
    m_modelViewProjectionLocation = glGetUniformLocation(
        m_shaderProgram, "modelViewProjection");
    m_normalTransformLocation = glGetUniformLocation(
        m_shaderProgram, "normalTransform");
    m_lightPositionLocation = glGetUniformLocation(
        m_shaderProgram, "lightPosition");
    m_lightIntensityLocation = glGetUniformLocation(
        m_shaderProgram, "lightIntensity");
    m_timeLocation = glGetUniformLocation(
        m_shaderProgram, "time");
    m_colorLocation = glGetUniformLocation(
        m_shaderProgram, "color");

    m_vertPositionLocation = glGetAttribLocation(
        m_shaderProgram, "vertPosition");
    m_vertNormalLocation = glGetAttribLocation(
        m_shaderProgram, "vertNormal");
    m_vertColorLocation = glGetAttribLocation(
        m_shaderProgram, "vertColor");
    m_vertTexCoordLocation = glGetAttribLocation(
        m_shaderProgram, "vertTexCoord");
    m_vertVelocityLocation = glGetAttribLocation(
        m_shaderProgram, "vertVelocity");
    m_vertStartTimeLocation = glGetAttribLocation(
        m_shaderProgram, "vertStartTime");

    m_texture0 = glGetUniformLocation(
        m_shaderProgram, "texture0");
  }
}

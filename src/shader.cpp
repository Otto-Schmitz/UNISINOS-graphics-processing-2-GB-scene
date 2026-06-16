#include "shader.h"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>

static std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file) return {};
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

static unsigned int compileStage(unsigned int type, const char* source, const char* label) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  int ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[2048];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    std::cerr << "Erro no shader " << label << ":\n" << log << "\n";
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

unsigned int loadShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
  const std::string vs = readFile(vertexPath);
  const std::string fs = readFile(fragmentPath);
  if (vs.empty() || fs.empty()) {
    std::cerr << "Falha ao ler shaders:\n  " << vertexPath << "\n  " << fragmentPath << "\n";
    return 0;
  }

  const unsigned int vert = compileStage(GL_VERTEX_SHADER, vs.c_str(), "vertex");
  const unsigned int frag = compileStage(GL_FRAGMENT_SHADER, fs.c_str(), "fragment");
  if (!vert || !frag) return 0;

  const unsigned int program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);
  glDeleteShader(vert);
  glDeleteShader(frag);

  int ok = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[2048];
    glGetProgramInfoLog(program, sizeof(log), nullptr, log);
    std::cerr << "Erro ao linkar programa:\n" << log << "\n";
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

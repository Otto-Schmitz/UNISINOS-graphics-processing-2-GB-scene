#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Material {
  std::string name;
  glm::vec3 ka{0.2f};
  glm::vec3 kd{0.8f};
  glm::vec3 ks{0.3f};
  float shininess = 32.f;
  GLuint diffuseTex = 0;
};

struct MeshGroup {
  std::string name;
  GLuint vao = 0;
  GLuint vbo = 0;
  int vertexCount = 0;
  int materialIndex = -1;
};

struct ObjModel {
  std::vector<MeshGroup> groups;
  std::vector<Material> materials;
};

GLuint createWhiteTexture();
GLuint loadTexture2D(const std::string& path);
GLuint loadCubemap(const std::vector<std::string>& faces);
ObjModel loadObjModel(const std::string& path, float defaultShininess);
void destroyObjModel(ObjModel& model);

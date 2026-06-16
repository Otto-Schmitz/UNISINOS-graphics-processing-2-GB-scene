#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

struct AnimationDef {
  float speed = 1.f;
  std::vector<glm::vec3> controlPoints;
};

struct ObjectDef {
  std::string filePath;
  glm::vec3 translation{0.f};
  glm::vec3 rotationDeg{0.f};
  glm::vec3 scale{1.f};
  std::optional<AnimationDef> animation;
};

struct LightDef {
  glm::vec3 position{0.f, 10.f, 0.f};
  glm::vec3 color{1.f};
  float intensity = 1.f;
};

struct SceneConfig {
  std::vector<ObjectDef> objects;
  std::vector<LightDef> lights;
  float defaultShininess = 32.f;
  glm::vec3 cameraPosition{0.f, 5.f, 15.f};
  float cameraYaw = -90.f;
  float cameraPitch = -15.f;
  float cameraSpeed = 10.f;
  float fov = 45.f;
  float nearPlane = 0.1f;
  float farPlane = 500.f;
};

SceneConfig loadSceneConfig(const std::string& path);

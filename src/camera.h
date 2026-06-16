#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class Camera {
 public:
  glm::vec3 position{0.f, 2.f, 8.f};
  glm::vec3 front{0.f, 0.f, -1.f};
  glm::vec3 up{0.f, 1.f, 0.f};
  glm::vec3 right{1.f, 0.f, 0.f};
  glm::vec3 worldUp{0.f, 1.f, 0.f};
  float yaw = -90.f;
  float pitch = 0.f;
  float movementSpeed = 8.f;
  float mouseSensitivity = 0.08f;

  Camera() = default;
  Camera(glm::vec3 startPos, glm::vec3 startUp, float startYaw, float startPitch);

  glm::mat4 viewMatrix() const;
  void processKeyboard(const std::string& direction, float deltaTime);
  void processMouse(float xoffset, float yoffset);

 private:
  void updateVectors();
};

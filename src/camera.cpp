#include "camera.h"

#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 startPos, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPos), worldUp(startUp), yaw(startYaw), pitch(startPitch) {
  updateVectors();
}

glm::mat4 Camera::viewMatrix() const {
  return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(const std::string& direction, float deltaTime) {
  const float speed = movementSpeed * deltaTime;
  if (direction == "FORWARD") position += front * speed;
  if (direction == "BACK") position -= front * speed;
  if (direction == "LEFT") position -= right * speed;
  if (direction == "RIGHT") position += right * speed;
  if (direction == "UP") position += worldUp * speed;
  if (direction == "DOWN") position -= worldUp * speed;
}

void Camera::processMouse(float xoffset, float yoffset) {
  yaw += xoffset * mouseSensitivity;
  pitch = std::clamp(pitch + yoffset * mouseSensitivity, -89.f, 89.f);
  updateVectors();
}

void Camera::updateVectors() {
  glm::vec3 dir;
  dir.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
  dir.y = std::sin(glm::radians(pitch));
  dir.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
  front = glm::normalize(dir);
  right = glm::normalize(glm::cross(front, worldUp));
  up = glm::normalize(glm::cross(right, front));
}

#include "catmull_rom.h"

#include <algorithm>
#include <cmath>

glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                     const glm::vec3& p3, float t) {
  return 0.5f * (2.0f * p1 + (-p0 + p2) * t +
                 (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * (t * t) +
                 (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * (t * t * t));
}

void updateCatmullRomPosition(const std::vector<glm::vec3>& controlPoints, float speed,
                              float deltaTime, float& animT, glm::vec3& position) {
  if (controlPoints.size() < 2) return;

  const int numSegments = static_cast<int>(controlPoints.size()) - 1;
  animT = std::fmod(animT + speed * deltaTime, static_cast<float>(numSegments));

  const int seg = static_cast<int>(animT);
  const float u = animT - static_cast<float>(seg);

  const int i0 = std::max(seg - 1, 0);
  const int i1 = seg;
  const int i2 = std::min(seg + 1, static_cast<int>(controlPoints.size()) - 1);
  const int i3 = std::min(seg + 2, static_cast<int>(controlPoints.size()) - 1);

  position = catmullRom(controlPoints[static_cast<size_t>(i0)],
                        controlPoints[static_cast<size_t>(i1)],
                        controlPoints[static_cast<size_t>(i2)],
                        controlPoints[static_cast<size_t>(i3)], u);
}

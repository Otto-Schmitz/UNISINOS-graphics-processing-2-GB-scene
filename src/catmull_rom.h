#pragma once

#include <glm/glm.hpp>
#include <vector>

glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                     const glm::vec3& p3, float t);

// Atualiza `position` ao longo dos pontos de controle (curva fechada se houver 3+ pontos).
void updateCatmullRomPosition(const std::vector<glm::vec3>& controlPoints, float speed,
                              float deltaTime, float& animT, glm::vec3& position);

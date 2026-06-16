#include "config.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

static glm::vec3 readVec3(const nlohmann::json& j) {
  return glm::vec3(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());
}

SceneConfig loadSceneConfig(const std::string& path) {
  std::ifstream file(path);
  if (!file) {
    std::cerr << "Não foi possível abrir " << path << "\n";
    std::exit(1);
  }

  nlohmann::json j;
  file >> j;

  SceneConfig cfg;

  if (j.contains("lighting")) {
    const auto& lighting = j["lighting"];
    cfg.defaultShininess = lighting.value("defaultQ", 32.f);
    for (const auto& light : lighting.at("sources")) {
      LightDef ld;
      ld.position = readVec3(light.at("position"));
      const glm::vec3 rgb(light["color"]["r"].get<float>() / 255.f,
                          light["color"]["g"].get<float>() / 255.f,
                          light["color"]["b"].get<float>() / 255.f);
      ld.intensity = light.value("intensity", 1.f);
      ld.color = rgb;
      cfg.lights.push_back(ld);
    }
  }

  if (j.contains("camera")) {
    const auto& cam = j["camera"];
    cfg.cameraPosition = readVec3(cam.at("position"));
    cfg.cameraYaw = cam.value("yaw", -90.f);
    cfg.cameraPitch = cam.value("pitch", 0.f);
    cfg.cameraSpeed = cam.value("speed", 10.f);
    cfg.fov = cam["frustum"].value("fov", 45.f);
    cfg.nearPlane = cam["frustum"].value("near", 0.1f);
    cfg.farPlane = cam["frustum"].value("far", 500.f);
  }

  for (const auto& obj : j.at("objects")) {
    ObjectDef def;
    def.filePath = obj.at("filePath").get<std::string>();
    def.translation = readVec3(obj.at("translation"));
    def.rotationDeg = readVec3(obj.at("rotation"));
    def.scale = readVec3(obj.at("scale"));

    if (obj.contains("animation") && obj["animation"].contains("controlPoints")) {
      AnimationDef anim;
      anim.speed = obj["animation"].value("speed", 1.f);
      for (const auto& pt : obj["animation"]["controlPoints"]) {
        anim.controlPoints.push_back(readVec3(pt));
      }
      if (anim.controlPoints.size() >= 2) {
        def.animation = anim;
      }
    }

    cfg.objects.push_back(std::move(def));
  }

  return cfg;
}

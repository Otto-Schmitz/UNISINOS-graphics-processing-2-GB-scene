#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "catmull_rom.h"
#include "config.h"
#include "obj_loader.h"
#include "paths.h"
#include "shader.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

constexpr int kMaxLights = 4;

struct SceneObject {
  std::string label;
  ObjModel model;
  glm::vec3 translation{0.f};
  glm::vec3 rotationDeg{0.f};
  glm::vec3 scale{1.f};
  std::optional<AnimationDef> animation;
  float animT = 0.f;
};

struct AppState {
  GLFWwindow* window = nullptr;
  Camera camera;
  std::vector<SceneObject> objects;
  std::vector<LightDef> lights;
  float defaultShininess = 32.f;
  float fov = 45.f;
  float nearPlane = 0.1f;
  float farPlane = 500.f;
  int selected = 0;
  bool firstMouse = true;
  float lastMouseX = 0.f;
  float lastMouseY = 0.f;

  unsigned int phongProgram = 0;
  unsigned int skyboxProgram = 0;
  unsigned int skyboxVAO = 0;
  unsigned int skyboxVBO = 0;
  unsigned int cubemapTex = 0;
  unsigned int whiteTex = 0;
};

AppState* gApp = nullptr;

glm::mat4 buildModelMatrix(const SceneObject& obj) {
  glm::mat4 model = glm::translate(glm::mat4(1.f), obj.translation);
  model = glm::rotate(model, glm::radians(obj.rotationDeg.z), glm::vec3(0, 0, 1));
  model = glm::rotate(model, glm::radians(obj.rotationDeg.y), glm::vec3(0, 1, 0));
  model = glm::rotate(model, glm::radians(obj.rotationDeg.x), glm::vec3(1, 0, 0));
  model = glm::scale(model, obj.scale);
  return model;
}

void setupSkyboxGeometry(float size, unsigned int& vao, unsigned int& vbo) {
  const float s = size * 0.5f;
  const float verts[] = {
      -s, s,  -s, -s, -s, -s, s,  -s, -s, s,  -s, -s, s,  s,  -s, -s, s,  -s,
      -s, -s, s,  -s, -s, -s, -s, s,  -s, -s, s,  -s, -s, s,  s,  -s, -s, s,
      s,  -s, -s, s,  -s, s,  s,  s,  s,  s,  s,  s,  s,  s,  -s, s,  -s, -s,
      -s, -s, s,  -s, s,  s,  s,  s,  s,  s,  s,  s,  s,  -s, s,  -s, -s, s,
      -s, s,  -s, s,  s,  -s, s,  s,  s,  s,  s,  s,  -s, s,  s,  -s, s,  -s,
      -s, -s, -s, -s, -s, s,  s,  -s, -s, s,  -s, -s, -s, -s, s,  s,  -s, s,
  };

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glBindVertexArray(0);
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
  if (height <= 0) height = 1;
  glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow*, double xpos, double ypos) {
  if (!gApp) return;
  const float x = static_cast<float>(xpos);
  const float y = static_cast<float>(ypos);
  if (gApp->firstMouse) {
    gApp->lastMouseX = x;
    gApp->lastMouseY = y;
    gApp->firstMouse = false;
  }
  const float dx = x - gApp->lastMouseX;
  const float dy = gApp->lastMouseY - y;
  gApp->lastMouseX = x;
  gApp->lastMouseY = y;
  gApp->camera.processMouse(dx, dy);
}

void processInput(GLFWwindow* window, float deltaTime) {
  if (!gApp) return;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) gApp->camera.processKeyboard("FORWARD", deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) gApp->camera.processKeyboard("BACK", deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) gApp->camera.processKeyboard("LEFT", deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) gApp->camera.processKeyboard("RIGHT", deltaTime);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) gApp->camera.processKeyboard("UP", deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    gApp->camera.processKeyboard("DOWN", deltaTime);
  }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
  if (!gApp || action != GLFW_PRESS) return;
  if (key == GLFW_KEY_LEFT && !gApp->objects.empty()) {
    gApp->selected = (gApp->selected - 1 + static_cast<int>(gApp->objects.size())) %
                     static_cast<int>(gApp->objects.size());
  }
  if (key == GLFW_KEY_RIGHT && !gApp->objects.empty()) {
    gApp->selected = (gApp->selected + 1) % static_cast<int>(gApp->objects.size());
  }
}

void drawSkybox(const AppState& app, const glm::mat4& projection, const glm::mat4& view) {
  glDepthMask(GL_FALSE);
  glUseProgram(app.skyboxProgram);
  const glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
  glUniformMatrix4fv(glGetUniformLocation(app.skyboxProgram, "uProjection"), 1, GL_FALSE,
                     glm::value_ptr(projection));
  glUniformMatrix4fv(glGetUniformLocation(app.skyboxProgram, "uView"), 1, GL_FALSE,
                     glm::value_ptr(viewNoTranslation));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, app.cubemapTex);
  glUniform1i(glGetUniformLocation(app.skyboxProgram, "uSkybox"), 0);
  glBindVertexArray(app.skyboxVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
}

void drawScene(const AppState& app, const glm::mat4& projection, const glm::mat4& view) {
  glUseProgram(app.phongProgram);

  const GLint locModel = glGetUniformLocation(app.phongProgram, "uModel");
  const GLint locMvp = glGetUniformLocation(app.phongProgram, "uMVP");
  const GLint locNormal = glGetUniformLocation(app.phongProgram, "uNormalMat");
  const GLint locViewPos = glGetUniformLocation(app.phongProgram, "uViewPos");
  const GLint locKa = glGetUniformLocation(app.phongProgram, "uKa");
  const GLint locKd = glGetUniformLocation(app.phongProgram, "uKd");
  const GLint locKs = glGetUniformLocation(app.phongProgram, "uKs");
  const GLint locShininess = glGetUniformLocation(app.phongProgram, "uShininess");
  const GLint locDiffuse = glGetUniformLocation(app.phongProgram, "uDiffuseMap");
  const GLint locNumLights = glGetUniformLocation(app.phongProgram, "uNumLights");
  const GLint locSelected = glGetUniformLocation(app.phongProgram, "uSelected");

  glUniform3fv(locViewPos, 1, glm::value_ptr(app.camera.position));
  const int lightCount = std::min(static_cast<int>(app.lights.size()), kMaxLights);
  glUniform1i(locNumLights, lightCount);
  for (int i = 0; i < lightCount; ++i) {
    const std::string idx = std::to_string(i);
    glUniform3fv(glGetUniformLocation(app.phongProgram, ("uLightPos[" + idx + "]").c_str()), 1,
                 glm::value_ptr(app.lights[static_cast<size_t>(i)].position));
    glUniform3fv(glGetUniformLocation(app.phongProgram, ("uLightColor[" + idx + "]").c_str()), 1,
                 glm::value_ptr(app.lights[static_cast<size_t>(i)].color));
    glUniform1f(glGetUniformLocation(app.phongProgram, ("uLightIntensity[" + idx + "]").c_str()),
                app.lights[static_cast<size_t>(i)].intensity);
  }

  for (size_t objIndex = 0; objIndex < app.objects.size(); ++objIndex) {
    const SceneObject& sceneObj = app.objects[objIndex];
    const glm::mat4 model = buildModelMatrix(sceneObj);
    const glm::mat4 mvp = projection * view * model;
    const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));

    glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(locMvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix3fv(locNormal, 1, GL_FALSE, glm::value_ptr(normalMat));
    glUniform1i(locSelected, static_cast<int>(objIndex) == app.selected ? 1 : 0);

    for (const MeshGroup& group : sceneObj.model.groups) {
      const Material* material = nullptr;
      if (group.materialIndex >= 0 &&
          group.materialIndex < static_cast<int>(sceneObj.model.materials.size())) {
        material = &sceneObj.model.materials[static_cast<size_t>(group.materialIndex)];
      }

      const Material fallback;
      const Material& mat = material ? *material : fallback;
      const GLuint tex = mat.diffuseTex ? mat.diffuseTex : app.whiteTex;

      glUniform3fv(locKa, 1, glm::value_ptr(mat.ka));
      glUniform3fv(locKd, 1, glm::value_ptr(mat.kd));
      glUniform3fv(locKs, 1, glm::value_ptr(mat.ks));
      glUniform1f(locShininess, mat.shininess > 0.f ? mat.shininess : app.defaultShininess);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);
      glUniform1i(locDiffuse, 0);

      glBindVertexArray(group.vao);
      glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
      glBindVertexArray(0);
    }
  }
}

}  // namespace

int main() {
  if (!glfwInit()) {
    std::cerr << "glfwInit falhou\n";
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1280, 720, "Visualizador de Cenas 3D", nullptr, nullptr);
  if (!window) {
    std::cerr << "glfwCreateWindow falhou\n";
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "gladLoadGLLoader falhou\n";
    return 1;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glClearColor(0.10f, 0.11f, 0.13f, 1.f);

  AppState app;
  app.window = window;
  gApp = &app;

  const std::string root(PG_DATA_ROOT);
  const SceneConfig config = loadSceneConfig(root + "/src/config.json");

  app.lights = config.lights;
  app.defaultShininess = config.defaultShininess;
  app.fov = config.fov;
  app.nearPlane = config.nearPlane;
  app.farPlane = config.farPlane;
  app.camera = Camera(config.cameraPosition, glm::vec3(0.f, 1.f, 0.f), config.cameraYaw,
                      config.cameraPitch);
  app.camera.movementSpeed = config.cameraSpeed;

  app.phongProgram = loadShaderProgram(root + "/shaders/phong.vert", root + "/shaders/phong.frag");
  app.skyboxProgram =
      loadShaderProgram(root + "/shaders/skybox.vert", root + "/shaders/skybox.frag");
  if (!app.phongProgram || !app.skyboxProgram) {
    return 1;
  }

  const std::vector<std::string> cubemapFaces = {
      root + "/assets/skybox/right.jpg", root + "/assets/skybox/left.jpg",
      root + "/assets/skybox/top.jpg",   root + "/assets/skybox/bottom.jpg",
      root + "/assets/skybox/front.jpg", root + "/assets/skybox/back.jpg",
  };
  app.cubemapTex = loadCubemap(cubemapFaces);
  if (!app.cubemapTex) {
    std::cerr << "Cubemap não carregado.\n";
    return 1;
  }
  app.whiteTex = createWhiteTexture();
  setupSkyboxGeometry(400.f, app.skyboxVAO, app.skyboxVBO);

  for (const ObjectDef& def : config.objects) {
    SceneObject sceneObj;
    sceneObj.label = def.filePath;
    sceneObj.translation = def.translation;
    sceneObj.rotationDeg = def.rotationDeg;
    sceneObj.scale = def.scale;
    sceneObj.animation = def.animation;
    sceneObj.model = loadObjModel(root + "/" + def.filePath, config.defaultShininess);
    if (sceneObj.model.groups.empty()) {
      std::cerr << "Objeto sem geometria: " << def.filePath << "\n";
      return 1;
    }
    app.objects.push_back(std::move(sceneObj));
  }

  std::cout << "Cena carregada: " << app.objects.size() << " objeto(s), "
            << app.lights.size() << " luz(es).\n";

  float lastFrame = static_cast<float>(glfwGetTime());
  while (!glfwWindowShouldClose(window)) {
    const float now = static_cast<float>(glfwGetTime());
    const float deltaTime = now - lastFrame;
    lastFrame = now;

    processInput(window, deltaTime);

    for (SceneObject& obj : app.objects) {
      if (obj.animation && obj.animation->controlPoints.size() >= 2) {
        updateCatmullRomPosition(obj.animation->controlPoints, obj.animation->speed, deltaTime,
                                 obj.animT, obj.translation);
      }
    }

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    const float aspect = width > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.f;
    const glm::mat4 projection =
        glm::perspective(glm::radians(app.fov), aspect, app.nearPlane, app.farPlane);
    const glm::mat4 view = app.camera.viewMatrix();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSkybox(app, projection, view);
    drawScene(app, projection, view);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  for (SceneObject& obj : app.objects) {
    destroyObjModel(obj.model);
  }
  if (app.skyboxVBO) glDeleteBuffers(1, &app.skyboxVBO);
  if (app.skyboxVAO) glDeleteVertexArrays(1, &app.skyboxVAO);
  if (app.cubemapTex) glDeleteTextures(1, &app.cubemapTex);
  if (app.whiteTex) glDeleteTextures(1, &app.whiteTex);
  glDeleteProgram(app.phongProgram);
  glDeleteProgram(app.skyboxProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

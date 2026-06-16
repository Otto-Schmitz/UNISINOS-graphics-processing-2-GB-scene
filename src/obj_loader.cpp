#include "obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

std::string parentDir(const std::string& path) {
  const size_t pos = path.find_last_of("/\\");
  return pos == std::string::npos ? "" : path.substr(0, pos + 1);
}

}  // namespace

GLuint createWhiteTexture() {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  const unsigned char white[] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

GLuint loadTexture2D(const std::string& path) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width = 0, height = 0, channels = 0;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
  if (!data) {
    std::cerr << "Falha ao carregar textura: " << path << "\n";
    glDeleteTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}

GLuint loadCubemap(const std::vector<std::string>& faces) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

  int width = 0, height = 0, channels = 0;
  for (size_t i = 0; i < faces.size(); ++i) {
    unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
    if (!data) {
      std::cerr << "Falha ao carregar face do cubemap: " << faces[i] << "\n";
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
      glDeleteTextures(1, &tex);
      return 0;
    }
    glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, GL_RGB, width,
                 height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return tex;
}

static std::vector<Material> loadMtl(const std::string& path, float defaultShininess) {
  std::vector<Material> materials;
  std::ifstream file(path);
  if (!file) {
    std::cerr << "Aviso: MTL não encontrado: " << path << "\n";
    return materials;
  }

  const std::string dir = parentDir(path);
  Material current;
  bool hasMaterial = false;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::string tag;
    ss >> tag;

    if (tag == "newmtl") {
      if (hasMaterial) materials.push_back(current);
      current = Material();
      current.shininess = defaultShininess;
      ss >> current.name;
      hasMaterial = true;
    } else if (tag == "Ka") {
      ss >> current.ka.r >> current.ka.g >> current.ka.b;
    } else if (tag == "Kd") {
      ss >> current.kd.r >> current.kd.g >> current.kd.b;
    } else if (tag == "Ks") {
      ss >> current.ks.r >> current.ks.g >> current.ks.b;
    } else if (tag == "Ns") {
      ss >> current.shininess;
    } else if (tag == "map_Kd") {
      std::string texName;
      std::getline(ss >> std::ws, texName);
      current.diffuseTex = loadTexture2D(dir + texName);
    }
  }

  if (hasMaterial) materials.push_back(current);
  return materials;
}

static void uploadGroup(const std::vector<float>& buffer, MeshGroup& group) {
  if (buffer.empty()) return;

  glGenVertexArrays(1, &group.vao);
  glGenBuffers(1, &group.vbo);
  glBindVertexArray(group.vao);
  glBindBuffer(GL_ARRAY_BUFFER, group.vbo);
  glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

  const GLsizei stride = 8 * sizeof(float);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));

  glBindVertexArray(0);
  group.vertexCount = static_cast<int>(buffer.size() / 8);
}

ObjModel loadObjModel(const std::string& path, float defaultShininess) {
  ObjModel model;
  std::ifstream file(path);
  if (!file) {
    std::cerr << "Erro ao abrir OBJ: " << path << "\n";
    return model;
  }

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  std::vector<float> buffer;

  const std::string objDir = parentDir(path);
  std::string currentGroupName = "default";
  int currentMaterialIndex = -1;

  auto flushGroup = [&]() {
    if (buffer.empty()) return;
    MeshGroup group;
    group.name = currentGroupName;
    group.materialIndex = currentMaterialIndex;
    uploadGroup(buffer, group);
    model.groups.push_back(group);
    buffer.clear();
  };

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::istringstream ss(line);
    std::string tag;
    ss >> tag;

    if (tag == "v") {
      glm::vec3 v;
      ss >> v.x >> v.y >> v.z;
      positions.push_back(v);
    } else if (tag == "vt") {
      glm::vec2 vt;
      ss >> vt.x >> vt.y;
      texcoords.push_back(vt);
    } else if (tag == "vn") {
      glm::vec3 vn;
      ss >> vn.x >> vn.y >> vn.z;
      normals.push_back(vn);
    } else if (tag == "mtllib") {
      std::string mtlFile;
      std::getline(ss >> std::ws, mtlFile);
      model.materials = loadMtl(objDir + mtlFile, defaultShininess);
    } else if (tag == "g" || tag == "o") {
      flushGroup();
      ss >> currentGroupName;
    } else if (tag == "usemtl") {
      flushGroup();
      std::string matName;
      ss >> matName;
      currentMaterialIndex = -1;
      for (size_t i = 0; i < model.materials.size(); ++i) {
        if (model.materials[i].name == matName) {
          currentMaterialIndex = static_cast<int>(i);
          break;
        }
      }
    } else if (tag == "f") {
      struct FaceVertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
      };
      std::vector<FaceVertex> face;

      std::string token;
      while (ss >> token) {
        int vi = -1, ti = -1, ni = -1;
        std::istringstream ts(token);
        std::string part;

        if (std::getline(ts, part, '/')) vi = part.empty() ? -1 : std::stoi(part) - 1;
        if (std::getline(ts, part, '/')) ti = part.empty() ? -1 : std::stoi(part) - 1;
        if (std::getline(ts, part, '/')) ni = part.empty() ? -1 : std::stoi(part) - 1;

        FaceVertex fv;
        fv.pos = vi >= 0 ? positions[static_cast<size_t>(vi)] : glm::vec3(0.f);
        fv.uv = ti >= 0 ? texcoords[static_cast<size_t>(ti)] : glm::vec2(0.f);
        fv.normal = ni >= 0 ? normals[static_cast<size_t>(ni)] : glm::vec3(0.f, 1.f, 0.f);
        face.push_back(fv);
      }

      auto pushVertex = [&](const FaceVertex& fv) {
        buffer.push_back(fv.pos.x);
        buffer.push_back(fv.pos.y);
        buffer.push_back(fv.pos.z);
        buffer.push_back(fv.normal.x);
        buffer.push_back(fv.normal.y);
        buffer.push_back(fv.normal.z);
        buffer.push_back(fv.uv.x);
        buffer.push_back(1.f - fv.uv.y);
      };

      for (size_t i = 1; i + 1 < face.size(); ++i) {
        pushVertex(face[0]);
        pushVertex(face[i]);
        pushVertex(face[i + 1]);
      }
    }
  }

  flushGroup();

  const GLuint fallbackTex = createWhiteTexture();
  for (auto& mat : model.materials) {
    if (!mat.diffuseTex) mat.diffuseTex = fallbackTex;
  }

  std::cout << "OBJ carregado: " << path << " (" << model.groups.size() << " grupos, "
            << model.materials.size() << " materiais)\n";
  return model;
}

void destroyObjModel(ObjModel& model) {
  std::vector<GLuint> uniqueTextures;
  for (const auto& mat : model.materials) {
    if (!mat.diffuseTex) continue;
    if (std::find(uniqueTextures.begin(), uniqueTextures.end(), mat.diffuseTex) ==
        uniqueTextures.end()) {
      uniqueTextures.push_back(mat.diffuseTex);
    }
  }

  for (auto& group : model.groups) {
    if (group.vbo) glDeleteBuffers(1, &group.vbo);
    if (group.vao) glDeleteVertexArrays(1, &group.vao);
    group.vbo = group.vao = 0;
    group.vertexCount = 0;
  }
  model.groups.clear();

  for (GLuint tex : uniqueTextures) {
    glDeleteTextures(1, &tex);
  }
  model.materials.clear();
}

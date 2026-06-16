cmake_minimum_required(VERSION 3.16)
project(scene_viewer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_path(GLM_INCLUDE_DIR glm/glm.hpp PATH_SUFFIXES glm)
if(NOT GLM_INCLUDE_DIR)
  message(FATAL_ERROR "GLM não encontrado (instale libglm-dev)")
endif()

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad")
add_library(glad STATIC "${GLAD_DIR}/src/glad.c")
target_include_directories(glad PUBLIC "${GLAD_DIR}/include")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/paths.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/paths.h"
  @ONLY
)

add_executable(scene_viewer
  src/main.cpp
  src/camera.cpp
  src/catmull_rom.cpp
  src/config.cpp
  src/obj_loader.cpp
  src/shader.cpp
)

target_include_directories(scene_viewer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_BINARY_DIR}/generated"
  "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb"
  ${GLM_INCLUDE_DIR}
  ${nlohmann_json_SOURCE_DIR}/single_include
)

target_link_libraries(scene_viewer PRIVATE
  OpenGL::GL
  glfw
  glad
)

if(UNIX AND NOT APPLE)
  target_link_libraries(scene_viewer PRIVATE dl)
endif()

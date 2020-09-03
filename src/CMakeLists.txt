
set(VULKAN_SDK_PATH "" CACHE PATH "Vulkan SDK path (include up to vulkan/[architecture])")

add_subdirectory(client/shaders)
add_subdirectory(client/textures)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(GLM REQUIRED glm)

add_executable(5d
  client/App.cpp
  client/GeometryChunk.cpp
  client/VulkanUtil.cpp
  client/WorldClient.cpp
  client/World.cpp
  server/WorldServer.cpp
  common/vec5.cpp
  common/Chunk.cpp
  common/util.cpp
  common/FlatWorldGenerator.cpp
  main.cpp
)

add_dependencies(5d spv texture)

target_include_directories(5d
  PUBLIC
  ${CMAKE_SOURCE_DIR}/lib/vendor
  ${GLFW_STATIC_INCLUDE_DIRS}
  ${GLM_STATIC_INCLUDE_DIRS}
  ${VULKAN_SDK_PATH}/include
)

target_link_directories(5d
  PUBLIC
  ${VULKAN_SDK_PATH}/lib
  ${GLFW_STATIC_LIBRARY_DIRS}
)

target_link_libraries(5d
  PUBLIC
  GameNetworkingSockets
  ${GLFW_STATIC_LIBRARIES}
  vulkan
)

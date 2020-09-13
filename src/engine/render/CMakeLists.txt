add_library(5DRender
  Vertex.cpp
  VulkanRenderer.cpp
  WorldView.cpp
)

target_include_directories(5DRender
  PUBLIC
  ${CMAKE_SOURCE_DIR}/lib/vendor
  ${GLM_STATIC_INCLUDE_DIRS}
  ${VULKAN_SDK_PATH}/include
)

target_link_directories(5DRender
  PUBLIC
  ${VULKAN_SDK_PATH}/lib
)

target_link_libraries(5DRender
  PUBLIC
  5DCore
  vulkan
)


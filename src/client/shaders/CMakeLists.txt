add_custom_command(
  OUTPUT frag.spv
  DEPENDS shader.frag
  COMMAND ${VULKAN_SDK_PATH}/bin/glslc ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag -o frag.spv
)

add_custom_command(
  OUTPUT vert.spv
  DEPENDS shader.vert
  COMMAND ${VULKAN_SDK_PATH}/bin/glslc ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert -o vert.spv
)

add_custom_target(spv DEPENDS frag.spv vert.spv)

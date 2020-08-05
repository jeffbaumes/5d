add_custom_command(
  OUTPUT merged.png
  COMMAND convert
    "\\(" ${CMAKE_CURRENT_SOURCE_DIR}/grass.png ${CMAKE_CURRENT_SOURCE_DIR}/stone.png +append "\\)"
    "\\(" ${CMAKE_CURRENT_SOURCE_DIR}/water.png "\\)"
    -background none -append merged.png
)

add_custom_target(texture DEPENDS merged.png)

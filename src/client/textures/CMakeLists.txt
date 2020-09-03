# add_custom_command(
#   OUTPUT merged.png
#   COMMAND convert
#     "\\(" ${CMAKE_CURRENT_SOURCE_DIR}/grass.png ${CMAKE_CURRENT_SOURCE_DIR}/stone.png +append "\\)"
#     "\\(" ${CMAKE_CURRENT_SOURCE_DIR}/water.png "\\)"
#     -background none -append merged.png
# )

add_custom_command(
  OUTPUT merged.png
  COMMAND convert
    "\\("
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/stone.png
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/dirt.png
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/grass.png
      +append
    "\\)"
    "\\("
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/wood.png
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/planks.png
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/leaves.png
      +append
    "\\)"
    "\\("
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/sand.png
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/water.png
      ${CMAKE_CURRENT_SOURCE_DIR}/smoothic/brick.png
      +append
    "\\)"
    -background none -append merged.png
)

add_custom_target(texture DEPENDS merged.png)

add_library(5DCore
  Chunk.cpp
  FlatChunkGenerator.cpp
  glm_util.cpp
  LocalChunkHandler.cpp
  types.cpp
  World.cpp
)

target_include_directories(5DCore
  PUBLIC
  ${GLM_STATIC_INCLUDE_DIRS}
)

target_link_libraries(5DCore
  PUBLIC
  pthread
)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLM REQUIRED glm)

add_subdirectory(core)
add_subdirectory(render)

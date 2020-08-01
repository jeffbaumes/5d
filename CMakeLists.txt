cmake_minimum_required(VERSION 3.5)
project(5d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(lib/vendor/GameNetworkingSockets)
add_subdirectory(src)

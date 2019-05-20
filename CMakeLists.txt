project(signaling_server)

cmake_minimum_required(VERSION 2.1)
SET(CMAKE_C_COMPILER g++)
add_compile_options(-std=c++11)
#aux_source_directory(. sources)
file(GLOB headers "session/*.h" "*.h")
file(GLOB sources "session/*.cpp" "*.cpp")

#add_library(${sources})
include_directories(
    .
)

add_executable(signaling_server ${sources})

project(signaling_server)

cmake_minimum_required(VERSION 3.1)

#aux_source_directory(. sources)
file(GLOB headers "*.h")
file(GLOB sources "*.cc" "./base/*.cc")

#add_library(${sources})
include_directories(
    .
    ./base
)

add_executable(signaling_server ${sources})
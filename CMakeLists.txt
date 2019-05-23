project(signaling_server)

cmake_minimum_required(VERSION 2.1)
SET(CMAKE_C_COMPILER g++)
SET(CMAKE_CXX_FLAGS "-g -w -O2 -pthread -lboost_system -lboost_date_time -lboost_thread")

add_compile_options(-std=c++11)
#aux_source_directory(. sources)
file(GLOB headers "*.h" "session/*.h")
file(GLOB sources "*.cpp" "session/*.cpp")

#add_library(${sources})
include_directories(
    .
    /home/ubuntu/boost
    /usr/include/boost
)
link_directories(
    /home/ubuntu/boost/stage/lib
)

add_executable(signal_server ${sources})

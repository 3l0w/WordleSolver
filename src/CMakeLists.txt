cmake_minimum_required(VERSION 3.10)

# set the project name
project(WordleSolver)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(WordleSolver SHARED solver.cpp)
target_include_directories(WordleSolver PUBLIC .)
target_link_libraries(WordleSolver ${CMAKE_THREAD_LIBS_INIT})


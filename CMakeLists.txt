cmake_minimum_required(VERSION 3.10)

include(FetchContent)

# For colors in the shell
FetchContent_Declare(termcolor
  GIT_REPOSITORY git://github.com/ikalnytskyi/termcolor.git
  GIT_TAG origin/master)
FetchContent_GetProperties(termcolor)

if(NOT termcolor_POPULATED)
  FetchContent_Populate(termcolor)
  add_subdirectory(${termcolor_SOURCE_DIR} ${termcolor_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# For the progress bar
FetchContent_Declare(indicators
  GIT_REPOSITORY git://github.com/p-ranav/indicators.git
  GIT_TAG origin/master
)
FetchContent_GetProperties(indicators)

if(NOT indicators_POPULATED)
  FetchContent_Populate(indicators)
  add_subdirectory(${indicators_SOURCE_DIR} ${indicators_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Prompt
FetchContent_Declare(replxx
  GIT_REPOSITORY git://github.com/AmokHuginnsson/replxx.git
  GIT_TAG origin/master
)
FetchContent_GetProperties(replxx)

if(NOT replxx_POPULATED)
  FetchContent_Populate(replxx)
  add_subdirectory(${replxx_SOURCE_DIR} ${replxx_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# set the project name
project(WordleSolver)

# add the executable
add_executable(WordleSolver wordlesolver.cpp solver.cpp solver.h)
target_link_libraries(WordleSolver PRIVATE termcolor)
target_link_libraries(WordleSolver PRIVATE indicators)
target_link_libraries(WordleSolver PRIVATE replxx)

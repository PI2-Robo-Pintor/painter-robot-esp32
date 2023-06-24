cmake_minimum_required(VERSION 3.16)

# set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(painter-robot-esp32-bddtest)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../main/bddTest/ StepMotorTests)
add_subdirectory(lib/Catch2)
add_executable(bddTest)
target_link_libraries(bddTest
   Catch2::Catch2WithMain
   StepMotorTest
)
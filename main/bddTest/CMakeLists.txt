add_library(
   StepMotor  STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../StepMotor.cpp)

add_library(StepMotorTest OBJECT StepMotorTest.cpp)

target_link_libraries(StepMotorTest PRIVATE Catch2::Catch2)
target_link_libraries(StepMotorTest PRIVATE StepMotor)
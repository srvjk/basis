
cmake_minimum_required (VERSION 3.0)

set (PROJECT 
    tests
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

set (CMAKE_CXX_STANDARD 14)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable (test_all ${HEADERS} ${SOURCES})

add_test (NAME Test1 COMMAND test_all WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set_tests_properties (Test1 PROPERTIES
  PASS_REGULAR_EXPRESSION "success"
  FAIL_REGULAR_EXPRESSION "fail"
)

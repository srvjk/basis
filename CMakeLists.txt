cmake_minimum_required (VERSION 3.12)

set (PROJECT basis)

project (${PROJECT})

cmake_policy(SET CMP0074 NEW)

set (SYSTEM_VERSION_MAJOR 1)
set (SYSTEM_VERSION_MINOR 1)

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

if (TARGET_PLATFORM STREQUAL "windows")
    add_definitions (-DPLATFORM_WINDOWS)
elseif (TARGET_PLATFORM STREQUAL "linux")
    add_definitions (-DPLATFORM_LINUX)
    add_compile_options (-Wno-trigraphs -Wall
                         -Wno-switch
                         -Wno-unused-function
                         -Wno-unused-variable
                         -Wno-sign-compare
                         -Wno-reorder
                         -Wno-parentheses
                         -Wno-unused-but-set-variable
    )
else ()
    message (FATAL_ERROR "Unknown or unsupported target platform!")
endif ()

message (STATUS "System: " ${CMAKE_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION})
message (STATUS "Processor: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
message (STATUS "Generator: " ${CMAKE_GENERATOR})

set (THIS_FILE_VERSION 1)

message (STATUS "CMakeLists file version is ${THIS_FILE_VERSION}")

message (STATUS "Top-level source dir: ${PROJECT_SOURCE_DIR}")
message (STATUS "Binary dir: ${PROJECT_BINARY_DIR}")

set (BIN_DIR_NAME BIN)
set (LIB_DIR_NAME LIB)

# set output path for executables ("exe" and ".dll" files)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR_NAME}/${TARGET_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# set output directories for libraries (".so" files)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR_NAME}/${TARGET_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# set output path for all static libraries (".lib"/".a", including import libraries for DLLs)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${LIB_DIR_NAME}/${TARGET_DIR})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

message (STATUS "Executable files for 'debug' will be placed to " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
message (STATUS "Executable files for 'release' will be placed to " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
message (STATUS ".so files for 'debug' will be placed to " ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
message (STATUS ".so files for 'release' will be placed to " ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE})

enable_testing()

add_subdirectory (SRC/sdk)
#add_subdirectory (SRC/workplace)
#add_subdirectory (SRC/control)
#add_subdirectory (SRC/modules/communicator)
#add_subdirectory (SRC/modules/dummy)
#add_subdirectory (SRC/modules/aquacontroller)
#add_subdirectory (SRC/modules/neuro)
#add_subdirectory (SRC/modules/neuroviewer)
add_subdirectory (SRC/tests)




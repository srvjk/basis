cmake_minimum_required (VERSION 3.4)

unset (QT_BINARY_DIR CACHE)
set (QT_BINARY_DIR ${ENV_PATH}/BIN)

set (PROJECT basis)

project (${PROJECT})

set (SYSTEM_VERSION_MAJOR 1)
set (SYSTEM_VERSION_MINOR 1)

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

set (PLATFORM_WINDOWS FALSE)
set (PLATFORM_LINUX FALSE)

if (TARGET_PLATFORM STREQUAL "win32")
    set (PLATFORM_WINDOWS TRUE)
    add_definitions (-DPLATFORM_WINDOWS)
else ()
    set (PLATFORM_LINUX TRUE)
    add_compile_options (-Wno-trigraphs -Wall
                         -Wno-switch
                         -Wno-unused-function
                         -Wno-unused-variable
                         -Wno-sign-compare
                         -Wno-reorder
                         -Wno-parentheses
                         -Wno-unused-but-set-variable
    )
endif ()

message (STATUS "System: " ${CMAKE_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION})
message (STATUS "Processor: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
message (STATUS "Generator: " ${CMAKE_GENERATOR})

set (THIS_FILE_VERSION 1)

message (STATUS "CMakeLists file version is ${THIS_FILE_VERSION}")

message (STATUS "Top-level source dir: ${PROJECT_SOURCE_DIR}")
message (STATUS "Binary dir: ${PROJECT_BINARY_DIR}")

set (CMAKE_FIND_ROOT_PATH ${ENV_PATH})
set (CMAKE_PREFIX_PATH ${ENV_PATH})
set (BOOST_ROOT ${BOOST_ROOT})

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

add_subdirectory (SRC/basis_sdk)
add_subdirectory (SRC/basis_workplace)
add_subdirectory (SRC/basis_modules/dummy)
add_subdirectory (SRC/basis_modules/observer)
add_subdirectory (SRC/basis_modules/neuro)



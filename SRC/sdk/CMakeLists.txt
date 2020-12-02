
set (PROJECT 
    basis
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DBASIS_LIB)

set (CMAKE_CXX_STANDARD 14)

find_package (Boost 1.74.0 REQUIRED)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories (${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})

add_library (${PROJECT} SHARED ${HEADERS} ${SOURCES})

target_link_libraries (${PROJECT} ${Boost_LIBRARIES})

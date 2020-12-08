set (PROJECT
    basis
    )

project (${PROJECT})

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

add_definitions (-DBASIS_LIB)

set (CMAKE_CXX_STANDARD 14)

if (PLATFORM_WINDOWS)
    find_package (Boost 1.74.0 REQUIRED)
elseif (PLATFORM_LINUX)
    find_package (Boost 1.74.0 REQUIRED filesystem)
endif ()

set (CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories (${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})

add_library (${PROJECT} SHARED ${HEADERS} ${SOURCES})

set (LINK_LIBRARIES ${Boost_LIBRARIES})
if (PLATFORM_LINUX)
    set (LINK_LIBRARIES ${LINK_LIBRARIES} dl)
endif ()

target_link_libraries (${PROJECT} ${LINK_LIBRARIES})

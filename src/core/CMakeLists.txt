set (PROJECT
    basis
    )

project (${PROJECT})

#file (GLOB_RECURSE HEADERS "*.h")
#file (GLOB_RECURSE SOURCES "*.cpp")

set (HEADERS ../../include/basis.h basis_private.h iterable.h)
set (SOURCES basis.cpp basis_test.cpp iterable.cpp)

add_definitions (-DBASIS_LIB)

set (CMAKE_CXX_STANDARD 14)

if (PLATFORM_WINDOWS)
    find_package (Boost 1.74.0 REQUIRED)
elseif (PLATFORM_LINUX)
    find_package (Boost 1.74.0 REQUIRED filesystem)
endif ()

set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_library (${PROJECT} SHARED ${HEADERS} ${SOURCES})

target_include_directories (${PROJECT} PRIVATE ../../include)
target_include_directories (${PROJECT} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories (${PROJECT} PRIVATE ${Boost_LIBRARY_DIRS})

set (LINK_LIBRARIES ${Boost_LIBRARIES})
if (PLATFORM_LINUX)
    set (LINK_LIBRARIES ${LINK_LIBRARIES} dl)
endif ()

target_link_libraries (${PROJECT} ${LINK_LIBRARIES})

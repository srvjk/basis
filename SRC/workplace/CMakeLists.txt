set (PROJECT 
    workplace
    )

project (${PROJECT})

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

set (CMAKE_CXX_STANDARD 14)

find_package (Boost 1.74.0 REQUIRED)

set (SFX "")
if (CONFIGURATION STREQUAL debug)
	set (SFX -d)
endif ()

include_directories (../sdk)
include_directories (${Boost_INCLUDE_DIRS})
link_directories (../../LIB/${CONFIGURATION})
link_directories (${Boost_LIBRARY_DIRS})

add_executable (${PROJECT} ${HEADERS} ${SOURCES})

target_link_libraries (${PROJECT} ${Boost_LIBRARIES} basis)

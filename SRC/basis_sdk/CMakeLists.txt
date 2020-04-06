cmake_minimum_required (VERSION 3.0)

set (PROJECT 
    basis
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DBASIS_LIB)

set (CMAKE_CXX_STANDARD 14)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.72.0)
else ()
	find_package (Boost 1.67.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories (${Boost_INCLUDE_DIR})

if (PLATFORM_WINDOWS)
	link_directories (${BOOST_LIBRARYDIR})
else ()
    #message (STATUS ">>> Boost library directory: " ${Boost_LIBRARY_DIR})
	#link_directories (${Boost_LIBRARY_DIR})
	set (Boost_LIBRARIES ${Boost_LIBRARIES} boost_system boost_filesystem dl)
endif (PLATFORM_WINDOWS)

add_library (${PROJECT} SHARED ${HEADERS} ${SOURCES})

target_link_libraries (${PROJECT} ${Boost_LIBRARIES})

cmake_minimum_required (VERSION 3.0)

set (PROJECT 
    workplace
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

set (CMAKE_CXX_STANDARD 14)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.72.0)
else ()
	find_package (Boost 1.67.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

include_directories (../sdk)
include_directories (${Boost_INCLUDE_DIR})

link_directories (../../LIB/${CONFIGURATION})

set (SFX "")
if (CONFIGURATION STREQUAL debug)
	set (SFX -d)
endif ()

if (PLATFORM_WINDOWS)
	link_directories (${BOOST_LIBRARYDIR})
else ()
	#link_directories (${Boost_LIBRARY_DIR})
	set (Boost_LIBRARIES ${Boost_LIBRARIES} boost_system boost_program_options)
endif (PLATFORM_WINDOWS)

add_executable (${PROJECT} ${HEADERS} ${SOURCES})

set_target_properties(${PROJECT} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt")
set_target_properties(${PROJECT} PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:msvcrt")

target_link_libraries (${PROJECT} ${Boost_LIBRARIES} basis)

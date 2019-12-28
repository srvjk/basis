cmake_minimum_required (VERSION 3.0)

set (PROJECT 
    basis_workplace
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

set (RESOURCES_DIR
    resources
    )

set (RESOURCES
    ${RESOURCES_DIR}/resources.qrc
    )

project (${PROJECT})

set (CMAKE_CXX_STANDARD 14)

find_package (Qt5Core)
if (PLATFORM_WINDOWS)
	find_package (Boost 1.68.0)
else ()
	find_package (Boost 1.62.0 REQUIRED COMPONENTS system dll)
endif (PLATFORM_WINDOWS)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

include_directories (../basis_sdk)
include_directories (${Boost_INCLUDE_DIR})
include_directories ("D:/projects/SFML-2.5.1/include")

link_directories (../../LIB/${CONFIGURATION})
link_directories ("D:/projects/SFML-2.5.1/lib")

set (SFX "")
if (CONFIGURATION STREQUAL debug)
	set (SFX -d)
endif ()

set (SFML_LIBRARIES sfml-system${SFX} sfml-window${SFX} sfml-graphics${SFX})

if (PLATFORM_WINDOWS)
	link_directories (${BOOST_LIBRARYDIR})
else ()
	link_directories (${Boost_LIBRARY_DIR})
	set (Boost_LIBRARIES ${Boost_LIBRARIES} boost_system)
endif (PLATFORM_WINDOWS)

qt5_add_resources (QRC_SOURCES ${RESOURCES})

add_executable (${PROJECT} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})

qt5_use_modules (${PROJECT} Core)

set_target_properties(${PROJECT} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt")
set_target_properties(${PROJECT} PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB:msvcrt")

target_link_libraries (${PROJECT} ${QT_LIBRARIES} ${Boost_LIBRARIES} ${SFML_LIBRARIES} basis)

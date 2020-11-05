
set (PROJECT
    console
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

set (CMAKE_FIND_ROOT_PATH ${ENV_PATH})
set (CMAKE_PREFIX_PATH ${ENV_PATH})

add_definitions (-DCONSOLE_LIB)

find_package(Qt5Core)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.72.0)
else ()
	find_package (Boost 1.67.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

include_directories (../../basis_sdk)
include_directories (${Boost_INCLUDE_DIR})

link_directories (../../../LIB/${CONFIGURATION})

add_library (${PROJECT} MODULE ${HEADERS} ${SOURCES} ${MOC_SOURCES})

qt5_use_modules (${PROJECT} Core)

target_link_libraries(${PROJECT} basis ${QT_LIBRARIES})

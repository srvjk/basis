
set (PROJECT
    aquacontroller
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DAQUACONTROLLER_LIB)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.72.0)
else ()
	find_package (Boost 1.67.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

# OpenGL
find_package (OpenGL REQUIRED)
if (NOT OPENGL_FOUND)
    message (ERROR "OpenGL not found!")
else ()
    message (STATUS "OpenGL found.")
endif (NOT OPENGL_FOUND)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories (../../sdk)
include_directories (${Boost_INCLUDE_DIRS})
include_directories (../../../TOOLS/sfml/include)

link_directories (${Boost_LIBRARY_DIRS})
link_directories (../../../LIB/${CONFIGURATION})
link_directories (../../../TOOLS/sfml/lib)

if (CONFIGURATION STREQUAL debug)
    set (SFML_SFX -d)
endif ()

set (SFML_LIBS
    sfml-graphics${SFML_SFX}
    sfml-window${SFML_SFX}
    sfml-system${SFML_SFX}
    )

add_library (${PROJECT} MODULE ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} basis ${Boost_LIBRARIES} ${OPENGL_LIBRARIES} ${SFML_LIBS})

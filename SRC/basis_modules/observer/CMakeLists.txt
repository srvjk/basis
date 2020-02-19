
set (PROJECT
    observer
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DOBSERVER_LIB)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.68.0)
else ()
	find_package (Boost 1.62.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

# OpenGL
find_package (OpenGL REQUIRED)
if (NOT OPENGL_FOUND)
    message (ERROR "OpenGL not found!")
else ()
    message (STATUS "OpenGL found.")
endif (NOT OPENGL_FOUND)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

add_definitions (-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

include_directories (../../basis_sdk)
include_directories (${Boost_INCLUDE_DIR})

link_directories (../../../LIB/${CONFIGURATION})

if (${PLATFORM_WINDOWS})
    # в Windows библиотеки GLFW и GL/GLEW подключаются отдельно (в Linux - через менеджер пакетов)
    link_directories (glfw GL)
    set (GLEW_LIB glew32)
    set (GLFW_LIB glfw3)
else ()
    set (GLEW_LIB GLEW)
    set (GLFW_LIB glfw)
endif (${PLATFORM_WINDOWS})

add_library (${PROJECT} MODULE ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} ${OPENGL_LIBRARIES} ${GLEW_LIB} ${GLFW_LIB} basis)
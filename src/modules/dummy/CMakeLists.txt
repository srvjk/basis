set (PROJECT
    dummy
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DDUMMY_LIB)

find_package (Boost 1.74.0 REQUIRED)

include_directories (../../../include)
include_directories (${Boost_INCLUDE_DIRS})

add_library (${PROJECT} MODULE ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} basis)

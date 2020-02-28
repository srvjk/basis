
set (PROJECT
    dummy
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DDUMMY_LIB)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.68.0)
else ()
	find_package (Boost 1.62.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

include_directories (../../basis_sdk)
include_directories (${Boost_INCLUDE_DIR})

link_directories (../../../LIB/${CONFIGURATION})

add_library (${PROJECT} MODULE ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} basis)
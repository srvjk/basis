
set (PROJECT
    communicator
    )

file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE SOURCES "*.cpp")

project (${PROJECT})

add_definitions (-DCOMMUNICATOR_LIB)

if (PLATFORM_WINDOWS)
	find_package (Boost 1.72.0)
else ()
	find_package (Boost 1.67.0 REQUIRED COMPONENTS system)
endif (PLATFORM_WINDOWS)

find_package (nng REQUIRED)

include_directories (../../sdk)
include_directories (${Boost_INCLUDE_DIR})
include_directories (${NNG_INCLUDE_DIRS})

link_directories (${Boost_LIBRARY_DIRS})
link_directories (../../../LIB/${CONFIGURATION})

add_library (${PROJECT} MODULE ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} basis ${Boost_LIBRARIES} ${NNG_LIBRARY})

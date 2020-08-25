cmake_minimum_required(VERSION 3.12)
project(BuildUs
    VERSION 1.0
    LANGUAGES C CXX
    DESCRIPTION "Build system")

set(Boost_USE_STATIC_LIBS on)
set(Boost_USE_MULTITHREADED ON)

#
## To include the yaml-cpp parsing
##
include(FetchContent)
FetchContent_Declare(
		yaml-cpp
		GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
		GIT_SHALLOW	ON
		GIT_TAG yaml-cpp-0.6.3
	)
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
	message(STATUS "Populating yaml-cpp...")
	FetchContent_Populate(yaml-cpp)
	# Add here options for yaml-cpp building
	set(YAML_CPP_BUILD_TESTS OFF)
	add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
	message(STATUS "Done.")
endif()

include_directories(BEFORE SYSTEM ${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR}/include)


if (UNIX)
find_package(ZLIB REQUIRED) 
find_package(Boost 1.71.0 REQUIRED COMPONENTS iostreams filesystem)
else ()
find_package(Boost 1.71.0 REQUIRED COMPONENTS iostreams filesystem zlib)
endif ()
		
set(CXX_STANDARD 17)

add_subdirectory(buildussystem)
add_subdirectory(tests)

add_executable(BuildUs buildus.cpp)
target_link_libraries(BuildUs yaml-cpp Lib)



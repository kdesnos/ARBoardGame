cmake_minimum_required(VERSION 2.6)

# *******************************************
# ************* CMake Content ***************
# *******************************************
# This CMake create a workspace containing the following projects
# 
# Programs
#  - ARBoardGame.exe

project(ARBoardGame)

# Set to 1 to activate debug compilation (0 for release)
set(DEBUG 1)


if(${DEBUG})
        MESSAGE("Generate Debug project")
        set(CMAKE_BUILD_TYPE Debug)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
else()
        MESSAGE("Generate Release project")
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
endif()



# Location of libraries
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# *******************************************
# ********** ARBoardGame.exe  ***************
# *******************************************
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(
	GLOB_RECURSE
	header_files
	./include/*.hpp
)

file(
	GLOB_RECURSE
	source_files
	./src/*.cpp
)
add_executable( ARBoardGame ${source_files} ${header_files})
# target_link_libraries(ARBoardGame)

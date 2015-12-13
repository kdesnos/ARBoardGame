cmake_minimum_required(VERSION 2.8.11)

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
# ************** QT5 LIBRARY ****************
# *******************************************
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

# *******************************************
# *********** OpenCV LIBRARY ****************
# *******************************************

find_package( OpenCV REQUIRED )

file(GLOB
     OpenCV_DLL
     ${OpenCV_DIR}/bin/*.dll
)

# Copy all DLLs in output directory
MESSAGE("Copy DLLs into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(COPY ${OpenCV_DLL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


# *******************************************
# ********** ARBoardGame.exe  ***************
# *******************************************
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${Qt5Widgets_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

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
target_link_libraries(ARBoardGame Qt5::Widgets ${OpenCV_LIBS})



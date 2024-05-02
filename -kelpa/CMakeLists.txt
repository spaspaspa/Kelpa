#[[
	CMakeLists.txt
	@Path: ./CMakeLists.txt
	
cd ./Build
cmake ..
make

./Bin/x64/Debug/test


]]

cmake_minimum_required(VERSION 3.15)
project(kelpa
VERSION 1
DESCRIPTION "A Server Framwork"
HOMEPAGE_URL "https://github.com/RFoe"
LANGUAGE "CXX"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#最好用嵌套的CMAKE(add_subdirectory)
file(GLOB_RECURSE SRC  
    "${PROJECT_SOURCE_DIR}/Src/*.hpp"    
    "${PROJECT_SOURCE_DIR}/Src/*.h"  
    "${PROJECT_SOURCE_DIR}/Src/*.cc"  
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Bin/x64/Debug)

add_library(kelpa_shared SHARED ${SRC})
add_library(kelpa_static STATIC ${SRC})

include_directories(${PROJECT_SOURCE_DIR}/Include)
link_directories(${PROJECT_SOURCE_DIR}/Lib)

add_executable(test ${PROJECT_SOURCE_DIR}/Tests/test.cc)

target_link_libraries(test kelpa_shared wsock32 dbghelp)







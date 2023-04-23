cmake_minimum_required (VERSION 3.8)
project(xtl LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" 
  OR "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC" )
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()


include_directories("../")
set (ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../")
file(GLOB HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../xtl/*.h")
add_executable (xtl "playground.cpp" ${HEADER_FILES})

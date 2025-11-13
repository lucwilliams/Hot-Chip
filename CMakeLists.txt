cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
project(Hot-Chip)

# Find all *.cpp files in src/ recursively
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Enable warnings.
# Only optimise for release and only use debug symbols for debug builds.
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Werror
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-g -O0>
)

if (WIN32)
    set(SDL2_PATH "lib/SDL2-2.32.8/x86_64-w64-mingw32")
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
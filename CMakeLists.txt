cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
project(Hot-Chip)

# Find all *.cpp files in src/ recursively
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if (WIN32)
    set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2-2.32.8/x86_64-w64-mingw32")

    # Build for Windows with statically linked libraries for portability
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

    # Use Windows CUI for output
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")

    # Copy SDL2.dll into project root if it doesn't exist
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_PATH}/bin/SDL2.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll"
    )
endif()

# Enable warnings.
# Only optimise for release and only use debug symbols for debug builds.
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3 -Wall -Werror -Wextra>
    $<$<CONFIG:Debug>:-g -DDEBUG -O0>
)

# Output executable to project root
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
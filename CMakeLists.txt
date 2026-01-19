cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Hot-Chip)

# Find all *.cpp files in src/ recursively
file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS
    src/*.cpp
)

# Add ImGUI files for compilation
file(GLOB IMGUI_SOURCE CONFIGURE_DEPENDS
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_sdl2.cpp
    lib/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

# Add SDL2 files for compilation
add_subdirectory(lib/SDL2)

# Add NFDe files for compilation (file browser)
add_subdirectory(lib/NFDe)

add_executable(${PROJECT_NAME} ${SOURCE} ${IMGUI_SOURCE})

# Enable warnings
# Only optimise for release and only use debug symbols for debug builds
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3 -Wall -Werror -Wextra>
    $<$<CONFIG:Debug>:-g -DDEBUG -O0>
)

# Output executable to project root
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Make header files accessible across all .cpp files
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/SDL2/include
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${CMAKE_SOURCE_DIR}/lib/imgui-addons
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends
)

if (WIN32)
    # Use Windows CUI for output
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")

    # Copy SDL2.dll into project root if it doesn't exist.
    # Credit: https://github.com/libsdl-org/SDL/issues/6399#issuecomment-1280612287
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        VERBATIM
    )

    # Only link SDL2main on Windows
    # Link WinMM for timeBeginPeriod() function
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main winmm)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 nfd)

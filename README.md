# Hot-Chip | CHIP-8 Interpreter

Hot-Chip is a work-in-progress interpreter for the CHIP-8 programming language.

### Development progress:
    - Wrapper class for handling SDL window functionality ✔
    - Abstract registers and program memory (incl. stack and font data) ✔
    - Implement simple instructions to set/read registers and draw to display
    - Decode binary program files and execute instructions
    - Long term: Timers, input, sound and all remaining instructions.

The end goal for the program is a fully functional cross-platform CHIP-8 interpreter that can be compiled to Windows, Linux and macOS.

### Dependencies
Hot-Chip uses [SDL2](https://www.libsdl.org/) (licensed under the [zlib license](https://www.libsdl.org/license.php)). 

You will need to install SDL2 to build this project:
- **Windows (MinGW):** 
  - Download *SDL2-devel-2.32.8-mingw.zip* from the [SDL releases](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8).
  - Extract into `lib/` and compile with cmake.

SDL2 CMake build script provided by [tcbrindle](https://github.com/tcbrindle/sdl2-cmake-scripts) under the 2-Clause BSD Licence.
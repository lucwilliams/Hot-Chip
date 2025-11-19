# Hot-Chip | CHIP-8 Interpreter

Hot-Chip is a work-in-progress interpreter for the CHIP-8 programming language.

![IBM splash logo running in Hot-Chip](assets/IBM%20Splash.png)

### Development progress:
    - Wrapper class for handling SDL window functionality ✔
    - Abstract registers and program memory (incl. stack and font data) ✔
    - Decode binary ROM files and execute instructions ✔
    - Implement simple instructions to set/read registers and draw to display ✔
    - Long term: Timers, input, sound and all remaining instructions.

The end goal for the program is a fully functional cross-platform CHIP-8 interpreter that can be compiled to Windows, Linux and macOS.

### Dependencies
Hot-Chip uses [SDL2](https://www.libsdl.org/) (licensed under the [zlib license](https://www.libsdl.org/license.php)). 

You will need to install SDL2 to build this project:
- **Windows (MinGW):** 
  - Download *SDL2-devel-2.32.8-mingw.zip* from the [SDL releases](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8).
  - Extract into `lib/` in project root.

- **Linux:**
  - Install libSDL2 development binaries using your package manager.
  - e.g. `sudo apt install libsdl2-dev` for Ubuntu/Debian.

SDL2 CMake build script provided by [tcbrindle](https://github.com/tcbrindle/sdl2-cmake-scripts) under the 2-Clause BSD Licence.

### Build
Substitute release for debug in either command to compile in debug mode.

**Windows:**
```
cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -- -j
```

**Linux:**
```
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -- -j
```

### Run
**Windows:** `Hot-Chip.exe roms/ibm.ch8`

**Linux:** `./Hot-Chip roms/ibm.ch8`

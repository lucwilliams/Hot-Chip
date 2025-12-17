# Hot-Chip | CHIP-8 Interpreter

Hot-Chip is a work-in-progress interpreter for the CHIP-8 programming language.

![IBM splash logo running in Hot-Chip](assets/IBM%20Splash.png)

### Development progress:
    - Wrapper class for handling SDL window functionality ✔
    - Abstract registers and program memory (incl. stack and font data) ✔
    - Decode binary ROM files and execute instructions ✔
    - Implement simple instructions to set/read registers and draw to display ✔
    - Emulated timers and sound ✔ 
    - Complete initial draft implementation of full instruction set ✔
    - Create debug tooling using ImGUI for UI (WIP)
    - Debug and test all instructions for correct behaviour
    - Long term: 3DS port? 👀

The end goal for the program is a fully functional cross-platform CHIP-8 interpreter that can be compiled to Windows, Linux and macOS.

### Dependencies
Hot-Chip uses [SDL2](https://www.libsdl.org/) (licensed under the [zlib license](https://www.libsdl.org/license.php)).\
You will need to install SDL2 to build this project:
- **Windows (MinGW):** 
  - Download *SDL2-devel-2.32.8-mingw.zip* from the [SDL releases](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8).
  - Extract into `lib/` in project root.

- **Linux:**
  - Install libSDL2 development binaries using your package manager.
  - e.g. `sudo apt install libsdl2-dev` for Ubuntu/Debian.

- **MacOS:**
  - Install SDL2 library files using [Homebrew](https://brew.sh/).
  - `brew install sdl2`

SDL2 CMake build script is provided by [tcbrindle](https://github.com/tcbrindle/sdl2-cmake-scripts) under the 2-Clause BSD License.\
[Dear ImGUI](https://github.com/ocornut/imgui) is used in this project under the MIT license.

### Build
Substitute release for debug in both commands to compile in debug mode. [MinGW](https://www.msys2.org/) is required for compilation on Windows.

**Windows:**
```
cmake -S . -B build/release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build/release -- -j
```

**Linux/MacOS:**
```
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release -- -j
```

### Run
Substitute ibm.ch8 for Chip-8 ROM of your choice. 
See [Timendus Tests](https://github.com/Timendus/chip8-test-suite/tree/main/bin) to download ROMs for testing.

**Windows:** `Hot-Chip.exe ibm.ch8`

**Linux/MacOS:** `./Hot-Chip ibm.ch8`

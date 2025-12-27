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
Hot-Chip uses [SDL2](https://www.libsdl.org/) under the [zlib](https://www.libsdl.org/license.php) license and [Dear ImGUI](https://github.com/ocornut/imgui), under the [MIT](https://github.com/ocornut/imgui/blob/master/LICENSE.txt) license.

### Clone repository and dependencies:
```shell
git clone --recursive https://github.com/lucwilliams/Hot-Chip.git
cd Hot-Chip
```

### Build
Substitute release for debug in both commands to compile in debug mode. Requires [ninja](https://ninja-build.org/) to be installed.

Compiles on Windows, Linux and macOS.

```shell
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
```

### Run
Substitute ibm.ch8 for Chip-8 ROM of your choice. 
See [Timendus Tests](https://github.com/Timendus/chip8-test-suite/tree/main/bin) to download ROMs for testing.

**Windows:** `Hot-Chip.exe ibm.ch8`

**Linux/MacOS:** `./Hot-Chip ibm.ch8`

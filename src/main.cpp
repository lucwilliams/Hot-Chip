#include "interpreter/Chip8.h"

// Compile with -DDEBUG for debug output
#ifdef DEBUG
    constexpr bool debugEnabled = true;
#else
    constexpr bool debugEnabled = false;
#endif

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string fileName{argv[1]};

        Window window = Window();
        Chip8 interpreter = Chip8(fileName, window, debugEnabled);

        interpreter.start();
    } else {
        std::cout << "No ROM provided." << std::endl;
    }

    return 0;
}
#include "interpreter/Chip8.h"

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string fileName{argv[1]};

        // Create a window to use as a display
        MainWindow window = MainWindow();

        // Create a CHIP-8 interpreter with window passed by reference
        Chip8 interpreter = Chip8(fileName, window);

        interpreter.start();
    } else {
        std::cout << "No ROM provided." << std::endl;
    }

    return 0;
}
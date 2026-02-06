#include "interpreter/Chip8.h"

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string ROMPath{argv[1]};

        /*
         * Create a window to use as a display.
         *
         * Window holds the ROMPath selected by the user,
         * Chip8 retrieves this on initialisation and when
         * a ROM change occurs.
         */
        MainWindow window = MainWindow(ROMPath);

        // Create a CHIP-8 interpreter with window passed by reference
        Chip8 interpreter = Chip8(window);

        interpreter.start();
    } else {
        std::cout << "No ROM provided." << std::endl;
    }

    return 0;
}
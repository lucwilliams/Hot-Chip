#include "interpreter/Chip8.h"

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string fileName{argv[1]};

        Window window = Window();
        Chip8 interpreter = Chip8(fileName, window);

        interpreter.start();
    } else {
        std::cout << "No ROM provided." << std::endl;
    }

    return 0;
}
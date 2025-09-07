#include "interpreter/Chip8.h"
#include "window/Window.h"
#include <iostream>

int main(int argc, char** argv) {
  if (argc > 1) {
    char* fileName = argv[1];

    Window window = Window();
    Chip8 interpreter = Chip8(fileName, window);

    interpreter.start();
  } else {
    std::cout << "No ROM provided." << std::endl;
  }

  return 0;
}
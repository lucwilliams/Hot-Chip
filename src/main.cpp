#include "window/Window.h"

int main(int argc, char** argv) {
  Window window = Window();

  SDL_Event event;
  bool running {true};

  // Window demo (smiley face)

  // Eyes
  window.flipPixel(25, 10);
  window.flipPixel(35, 10);

  // Grin
  window.flipPixel(23, 13);
  window.flipPixel(37, 13);

  // Smile
  for (int i {24}; i < 37; ++i)
    window.flipPixel(i, 14);

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
    }

    window.Draw();
  }

  return 0;
}
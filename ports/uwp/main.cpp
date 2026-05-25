#include <Windows.h>
#include "SDL2/SDL.h"

int bootstrap(int argc, char **argv) {
    return SDL_main(argc, argv);
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return SDL_WinRTRunApp(bootstrap, nullptr);
}

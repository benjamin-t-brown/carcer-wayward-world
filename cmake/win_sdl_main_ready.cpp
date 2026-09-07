// Windows-only shim for the game executable.
//
// Carcer's CMake strips MSYS2's SDL2main (and the pkg-config main->SDL_main
// rewrite) on Windows because the game's entry translation unit imports the
// sdl2w module rather than including <SDL.h>, which leaves SDL2main's
// `extern "C" SDL_main` reference unresolved. SDL2main would normally call
// SDL_SetMainReady() before the app runs, so do it here from a static
// initializer to keep SDL_Init() working on Windows.

#include <SDL.h>

namespace {
struct WinSdlMainReady {
  WinSdlMainReady() { SDL_SetMainReady(); }
};
const WinSdlMainReady win_sdl_main_ready;
}  // namespace

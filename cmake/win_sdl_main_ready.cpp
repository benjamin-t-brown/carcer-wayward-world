// Header-mode Carcer deliberately bypasses SDL2main on Windows so the entry
// point has the same shape on every platform. Preserve SDL2main's required
// initialization explicitly before the application starts.

#include <SDL.h>

namespace {
struct WinSdlMainReady {
  WinSdlMainReady() { SDL_SetMainReady(); }
};
const WinSdlMainReady win_sdl_main_ready;
} // namespace

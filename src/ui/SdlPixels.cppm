module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.core:SdlPixels;

export {

// --- from ui/SdlPixels.h ---
#if __has_include(<SDL2/SDL_pixels.h>)

#else

#endif

} // export

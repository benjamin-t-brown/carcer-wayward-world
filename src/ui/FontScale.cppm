module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.core:FontScale;
import sdl2w;
#include "macros.h"

export {

// --- from ui/FontScale.h ---
namespace ui {

int mapFontSizeToPixels(sdl2w::TextSize size);
sdl2w::TextSize mapPixelsToFontSize(int sizePx);
sdl2w::TextSize applyFontScale(sdl2w::TextSize baseSize, int fontScale);

} // namespace ui

} // export

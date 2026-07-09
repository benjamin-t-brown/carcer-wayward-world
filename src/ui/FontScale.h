#pragma once

#include "sdl2w/Defines.h"

namespace ui {

int mapFontSizeToPixels(sdl2w::TextSize size);
sdl2w::TextSize mapPixelsToFontSize(int sizePx);
sdl2w::TextSize applyFontScale(sdl2w::TextSize baseSize, int fontScale);

} // namespace ui

#pragma once

// The pinned SDL2W Animation.h owns a DynArray<Sprite> but intentionally only
// forward-declares Sprite. BMIN's current header implementation requires that
// element type to be complete when Animation is embedded in another value.
// Draw.h completes Sprite while also providing Animation. Keep that upstream
// header-order workaround in one place until SDL2W makes Animation.h
// independently self-contained.
#include "sdl2w/Draw.h"

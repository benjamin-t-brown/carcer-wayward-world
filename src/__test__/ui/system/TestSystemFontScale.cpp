#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "ui/FontScale.h"
#include <cassert>
#include <iostream>
#include "lib/Types.h"
#include <vector>

struct ScalePreviewRow {
  int scale = 0;
  sdl2w::TextSize renderedSize = sdl2w::TEXT_SIZE_16;
};

int main(int argc, char** argv) {
  using sdl2w::TextSize;

  const auto base10 = static_cast<TextSize>(10);
  assert(static_cast<int>(ui::applyFontScale(base10, -1)) == 10);
  assert(static_cast<int>(ui::applyFontScale(base10, 0)) == 10);
  assert(static_cast<int>(ui::applyFontScale(base10, 1)) == 12);

  // Validate fallback preset stepping still works for known presets.
  assert(static_cast<int>(ui::applyFontScale(sdl2w::TEXT_SIZE_20, -1)) == 18);
  assert(static_cast<int>(ui::applyFontScale(sdl2w::TEXT_SIZE_20, 1)) == 22);

  // Validate clamping at both ends.
  assert(static_cast<int>(ui::applyFontScale(sdl2w::TEXT_SIZE_10, -100)) == 10);
  assert(static_cast<int>(ui::applyFontScale(sdl2w::TEXT_SIZE_72, 100)) == 72);

  const sdl2w::TextSize previewBaseSize = sdl2w::TEXT_SIZE_10;
  const DynArray<int> previewScales{-1, 0, 1, 2, 3};
  const DynArray<String> previewFonts{"default", "alternate", "text", "title"};
  DynArray<ScalePreviewRow> previewRows;
  previewRows.reserve(previewScales.size());
  for (int scale : previewScales) {
    previewRows.pushBack(
        ScalePreviewRow{scale, ui::applyFontScale(previewBaseSize, scale)});
  }

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    (void)window;
    (void)store;
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    (void)store;
    auto& draw = window.getDraw();
    int headerY = 24;
    int firstRowY = 70;
    int rowHeight = 60;
    int firstColX = 30;
    int colWidth = 215;

    // Column headers
    for (size_t col = 0; col < previewFonts.size(); col++) {
      sdl2w::RenderTextParams headerParams;
      headerParams.fontName = "text";
      headerParams.fontSize = sdl2w::TEXT_SIZE_14;
      headerParams.x = firstColX + static_cast<int>(col) * colWidth;
      headerParams.y = headerY;
      headerParams.centered = false;
      headerParams.color = SDL_Color{255, 220, 120, 255};
      draw.drawText("Font: " + previewFonts[col], headerParams);
    }

    for (size_t rowIdx = 0; rowIdx < previewRows.size(); rowIdx++) {
      const auto& row = previewRows[rowIdx];
      int y = firstRowY + static_cast<int>(rowIdx) * rowHeight;
      for (size_t col = 0; col < previewFonts.size(); col++) {
        sdl2w::RenderTextParams params;
        params.fontName = previewFonts[col];
        params.fontSize = row.renderedSize;
        params.x = firstColX + static_cast<int>(col) * colWidth;
        params.y = y;
        params.centered = false;
        params.color = SDL_Color{255, 255, 255, 255};

        String label =
            "[" + bmin::toString(row.scale) + "/" +
            bmin::toString(static_cast<int>(row.renderedSize)) + "] Quick Brown Fox";
        draw.drawText(label, params);
      }
    }

    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{980, 420, "System Font Scale Preview"},
              _init,
              _updateRender);

  std::cout << "TestSystemFontScale passed\n";
  return 0;
}

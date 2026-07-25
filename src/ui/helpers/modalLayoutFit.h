#pragma once

namespace ui {

struct BaseStyle;

// How ModalStandard / ModalSmall map width/height props to the final rect.
// CappedCentered (default): treat width/height as window dims, then cap + center.
// FullBleed: use width/height as the modal size as-is (no cap/center).
enum class LayoutFit { CappedCentered, FullBleed };

// Landscape max size class. Portrait still uses near-full margins for both.
enum class ModalSizeClass { Standard, Small };

struct LayoutRect {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

// Portrait: near-full with small margins.
// Landscape Standard: ~500×min(500, h−50). Small: ~400×min(420, h−50).
LayoutRect computeCappedCenteredRect(int windowW,
                                     int windowH,
                                     ModalSizeClass sizeClass = ModalSizeClass::Standard);

// After a CappedCentered modal->setProps, sync the host page/minipage style to the
// fitted rect so getPos()+getDims() (often child[0] size) align with drawn controls.
// Call while host style still holds window width/height. Pass the same sizeClass
// the modal used.
void syncHostStyleToCappedCentered(BaseStyle& style,
                                   ModalSizeClass sizeClass = ModalSizeClass::Standard);

} // namespace ui

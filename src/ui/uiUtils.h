#pragma once

#include "UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep

namespace ui {

// Utility functions for UI operations

/**
 * Check if a point is inside a rectangle
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param rectX Rectangle X position
 * @param rectY Rectangle Y position
 * @param rectWidth Rectangle width
 * @param rectHeight Rectangle height
 * @return true if point is inside the rectangle
 */
inline bool
isInBounds(int x, int y, int rectX, int rectY, int rectWidth, int rectHeight) {
  return x >= rectX && x < rectX + rectWidth && y >= rectY && y < rectY + rectHeight;
}

/**
 * Check if a point is inside a UI element using getPos/getDims
 */
inline bool isInBounds(int x, int y, const UiElement* element) {
  if (!element)
    return false;

  auto [px, py] = element->getPos();
  auto dims = element->getDims();
  return isInBounds(x, y, px, py, dims.first, dims.second);
}

inline bool isInBounds(int x, int y, const UiElement& element) {
  auto [px, py] = element.getPos();
  auto dims = element.getDims();
  return isInBounds(x, y, px, py, dims.first, dims.second);
}

/**
 * Check if a point is inside a UI element (getDims already includes scale)
 */
inline bool isInBoundsScaled(int x, int y, const UiElement* element) {
  if (!element)
    return false;

  auto [px, py] = element->getPos();
  auto dims = element->getDims();
  return isInBounds(x, y, px, py, dims.first, dims.second);
}

inline bool isInBoundsScaled(int x, int y, const UiElement& element) {
  auto [px, py] = element.getPos();
  auto dims = element.getDims();
  return isInBounds(x, y, px, py, dims.first, dims.second);
}

} // namespace ui

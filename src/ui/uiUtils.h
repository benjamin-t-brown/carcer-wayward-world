#pragma once

#include "UiElement.h"
#include <SDL2/SDL_pixels.h>

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
inline bool isInBounds(int x, int y, int rectX, int rectY, int rectWidth, int rectHeight) {
  return x >= rectX && x < rectX + rectWidth && y >= rectY && y < rectY + rectHeight;
}

/**
 * Check if a point is inside a UI element using its style dimensions
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param element UI element to check against
 * @return true if point is inside the element
 */
inline bool isInBounds(int x, int y, const UiElement* element) {
  if (!element) return false;
  
  const auto& style = element->getStyle();
  return isInBounds(x, y, style.x, style.y, style.width, style.height);
}

/**
 * Check if a point is inside a UI element using its style dimensions (const version)
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param element UI element to check against
 * @return true if point is inside the element
 */
inline bool isInBounds(int x, int y, const UiElement& element) {
  const auto& style = element.getStyle();
  return isInBounds(x, y, style.x, style.y, style.width, style.height);
}

/**
 * Check if a point is inside a UI element with scaling applied
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param element UI element to check against
 * @return true if point is inside the scaled element
 */
inline bool isInBoundsScaled(int x, int y, const UiElement* element) {
  if (!element) return false;
  
  const auto& style = element->getStyle();
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);
  
  return isInBounds(x, y, style.x, style.y, scaledWidth, scaledHeight);
}

/**
 * Check if a point is inside a UI element with scaling applied (const version)
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param element UI element to check against
 * @return true if point is inside the scaled element
 */
inline bool isInBoundsScaled(int x, int y, const UiElement& element) {
  const auto& style = element.getStyle();
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);
  
  return isInBounds(x, y, style.x, style.y, scaledWidth, scaledHeight);
}

} // namespace ui
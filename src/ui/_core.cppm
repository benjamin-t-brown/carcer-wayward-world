module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <string_view>
#include <string>
#include <optional>

export module carcer.ui.core;
export import bmin.containers;
export import carcer.state;
import sdl2w;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/FontScale.h ---
namespace ui {

int mapFontSizeToPixels(sdl2w::TextSize size);
sdl2w::TextSize mapPixelsToFontSize(int sizePx);
sdl2w::TextSize applyFontScale(sdl2w::TextSize baseSize, int fontScale);

} // namespace ui

} // export

export {

// --- from ui/SdlPixels.h ---
#if __has_include(<SDL2/SDL_pixels.h>)

#else

#endif

} // export

export {

// --- from ui/colors.h ---
// IWYU pragma: keep

namespace ui {
struct Colors {
  static constexpr SDL_Color Transparent{0, 0, 0, 0};
  static constexpr SDL_Color Black{0, 0, 0, 255};
  static constexpr SDL_Color White{255, 255, 255, 255};
  static constexpr SDL_Color OffWhite{248, 248, 248, 255};
  static constexpr SDL_Color LightGrey{188, 183, 197, 255};
  static constexpr SDL_Color DarkGrey{75, 75, 75, 255};
  static constexpr SDL_Color Grey{128, 128, 128, 255};
  static constexpr SDL_Color Grey2{100, 100, 100, 255};
  static constexpr SDL_Color Red{225, 83, 74, 255};
  static constexpr SDL_Color Blue{57, 120, 168, 255};
  static constexpr SDL_Color LightBlue{66, 202, 253, 255};
  static constexpr SDL_Color DarkBlue{36, 63, 114, 255};
  static constexpr SDL_Color Purple{86, 64, 100, 255};
  static constexpr SDL_Color WarmGrey{80, 87, 107, 255};
  static constexpr SDL_Color Charcoal{17, 17, 17, 255};
  static constexpr SDL_Color Teal{0, 99, 92, 255};
  static constexpr SDL_Color Brown{160, 91, 83, 255};
  static constexpr SDL_Color DarkGreen{0, 95, 27, 255};
  static constexpr SDL_Color Green{57, 120, 68, 255};
  static constexpr SDL_Color ButtonModalGrey1{75, 75, 75, 255};
  static constexpr SDL_Color ButtonModalGrey2{100, 100, 100, 255};
  static constexpr SDL_Color ButtonModalGrey3{50, 50, 50, 255};
  static constexpr SDL_Color ButtonModalSelected{66, 202, 253, 255};
  static constexpr SDL_Color ButtonCloseRed{169, 59, 59, 255};
  static constexpr SDL_Color ButtonCloseRedBorder1{255, 139, 156, 255};
  static constexpr SDL_Color ButtonCloseRedBorder2{94, 54, 67, 255};
  static constexpr SDL_Color ButtonCloseRedHover{200, 70, 70, 255};
  static constexpr SDL_Color ButtonCloseRedActive{255, 0, 0, 255};
  static constexpr SDL_Color ButtonCloseTextWhite{255, 255, 255, 255};
  static constexpr SDL_Color ButtonCloseTextGrey{60, 60, 60, 255};
  static constexpr SDL_Color BorderModalStandard{45, 55, 64, 255};
  static constexpr SDL_Color BorderModalStandardLight{69, 83, 96, 255};
  static constexpr SDL_Color BorderModalStandardDark{23, 28, 33, 255};
  static constexpr SDL_Color ModalStandardBackground{248, 248, 248, 255};
  static constexpr SDL_Color ModalHeaderBackground{188, 183, 197, 255};
  static constexpr SDL_Color ButtonWorldActionDefault{100, 120, 140, 255};
  static constexpr SDL_Color ButtonWorldActionHover{120, 140, 160, 255};
  static constexpr SDL_Color ButtonWorldActionActive{80, 100, 120, 255};
  static constexpr SDL_Color ButtonWorldActionText{255, 255, 255, 255};
};
} // namespace ui

} // export

export {

// --- from ui/TextStyle.h ---
// IWYU pragma: keep

namespace ui {

enum class FontFamily { TEXT, TEXT_BOLD, DEFAULT, TITLE };

enum class TextAlign { LEFT_TOP, LEFT_CENTER, LEFT_BOTTOM, CENTER };

struct TextFontProps {
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
  TextAlign textAlign = TextAlign::LEFT_TOP;
};

enum class BaseFontConfig {
  MODAL_TEXT,
  MODAL_TEXT_BOLD,
  MODAL_TITLE,
  MODAL_CHOICE_TEXT,
  MODAL_BUTTON,
};

inline void setBaseFontConfig(TextFontProps& font, BaseFontConfig config) {
  switch (config) {
  case BaseFontConfig::MODAL_TEXT:
    font.fontFamily = FontFamily::TEXT;
    font.fontSize = sdl2w::TEXT_SIZE_20;
    font.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_TEXT_BOLD:
    font.fontFamily = FontFamily::TEXT_BOLD;
    font.fontSize = sdl2w::TEXT_SIZE_20;
    font.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_TITLE:
    font.fontFamily = FontFamily::TITLE;
    font.fontSize = sdl2w::TEXT_SIZE_24;
    font.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_CHOICE_TEXT:
    font.fontFamily = FontFamily::TEXT;
    font.fontSize = sdl2w::TEXT_SIZE_24;
    font.fontColor = Colors::Black;
    break;
  case BaseFontConfig::MODAL_BUTTON:
    font.fontFamily = FontFamily::TEXT;
    font.fontSize = sdl2w::TEXT_SIZE_20;
    font.fontColor = Colors::White;
    break;
  }
}

} // namespace ui

} // export

export {

// --- from ui/UiElement.h ---
// IWYU pragma: keep
 // IWYU pragma: keep

// prevents circular dependency
 // IWYU pragma: keep

namespace ui {

// Forward declaration
class UiElement;

// Base StateInterface that can dispatch actions
class StateInterface {
public:
  virtual ~StateInterface() = default;
  virtual void dispatchAction(const bmin::String& action, void* payload) = 0;
};

// Geometry-only style. Visual fields live on component props / private caches.
struct BaseStyle {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
  float scale = 1.0f;
};

class UiEventObserver {
public:
  virtual ~UiEventObserver() = default;
  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onClick(int x, int y, int button);
  virtual void onMouseWheel(int x, int y, int delta);
};

// Main UiElement base class
class UiElement : public state::StateManagerInterface,
                  public state::LayerManagerInterface {
protected:
  sdl2w::Window* window;
  UiElement* parent;
  bmin::DynArray<bmin::UniquePtr<UiElement>> children;
  std::optional<StateInterface*> stateInterface;
  BaseStyle style;
  bmin::String id;
  bmin::DynArray<bmin::UniquePtr<UiEventObserver>> eventObservers;
  bool shouldPropagateEventsToChildren = true;

public:
  bool isHovered = false;
  bool isClicked = false;
  // Constructor
  UiElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  virtual ~UiElement() = default;

  virtual UiElement* getChildById(std::string_view id);
  virtual void removeChildById(std::string_view id);

  // Layout API (style is private/protected; no public getStyle/setStyle)
  virtual void setPos(int x, int y);
  virtual void setScale(float scale);
  virtual std::pair<int, int> getPos() const;
  virtual const std::pair<int, int> getDims() const;

  // Id methods
  virtual void setId(const bmin::String& _id);
  virtual const bmin::String& getId() const;

  // Children methods
  virtual bmin::DynArray<bmin::UniquePtr<UiElement>>& getChildren();
  virtual const bmin::DynArray<bmin::UniquePtr<UiElement>>& getChildren() const;
  virtual void removeChildAtIndex(size_t index);
  virtual void addChild(UiElement* child);

  // Event handlers
  virtual bool checkMouseDownEvent(int mouseX,
                                   int mouseY,
                                   int button,
                                   bmin::DynArray<UiElement*> additionalElements = {});
  virtual bool checkMouseUpEvent(int mouseX,
                                 int mouseY,
                                 int button,
                                 bmin::DynArray<UiElement*> additionalElements = {});
  virtual bool checkHoverEvent(int mouseX,
                               int mouseY,
                               bmin::DynArray<UiElement*> additionalElements = {});
  virtual bool checkMouseWheelEvent(int mouseX,
                                    int mouseY,
                                    int delta,
                                    bmin::DynArray<UiElement*> additionalElements = {});
  virtual void checkResizeEvent(int width, int height);
  virtual void addEventObserver(UiEventObserver* observer);
  virtual void removeEventObserver(UiEventObserver* observer);

  // Build and render
  virtual void build();
  virtual void render(int dt);

  // Getters for window and parent
  sdl2w::Window* getWindow() const { return window; }
  UiElement* getParent() const { return parent; }
};

} // namespace ui

} // export

export {

// --- from ui/uiUtils.h ---
// IWYU pragma: keep

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

} // export

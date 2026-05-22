#pragma once

#include "lib/sdl2w/Defines.h"
#include "lib/sdl2w/Window.h"
#include "state/LayerManagerInterface.h"
#include "state/StateManagerInterface.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/colors.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

// prevents circular dependency
#include "state/AbstractAction.h" // IWYU pragma: keep

namespace ui {

// Forward declaration
class UiElement;

// Base StateInterface that can dispatch actions
class StateInterface {
public:
  virtual ~StateInterface() = default;
  virtual void dispatchAction(const std::string& action, void* payload) = 0;
};

// Enums for styling
enum class FontFamily { TEXT, ALTERNATE, TITLE };

enum class TextAlign { LEFT_TOP, LEFT_CENTER, LEFT_BOTTOM, CENTER };

// Base style structure - can be extended by specific UI elements
struct BaseStyle {
  // Position
  int x = 0;
  int y = 0;

  // Size
  int width = 0;
  int height = 0;

  // Scale
  float scale = 1.0f;

  // Text params
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
  TextAlign textAlign = TextAlign::LEFT_TOP;

  // List
  int lineSpacing = 0;
  SDL_Color lineBackgroundColor = SDL_Color{255, 255, 255, 0};
};

enum class BaseFontConfig {
  MODAL_TEXT,
  MODAL_TITLE,
  MODAL_CHOICE_TEXT,
  MODAL_BUTTON,
};

inline void setBaseFontConfig(BaseStyle& style, BaseFontConfig config) {
  switch (config) {
  case BaseFontConfig::MODAL_TEXT:
    style.fontFamily = FontFamily::TEXT;
    style.fontSize = sdl2w::TEXT_SIZE_16;
    style.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_TITLE:
    style.fontFamily = FontFamily::TITLE;
    style.fontSize = sdl2w::TEXT_SIZE_20;
    style.fontColor = Colors::White;
    break;
  case BaseFontConfig::MODAL_CHOICE_TEXT:
    style.fontFamily = FontFamily::TEXT;
    style.fontSize = sdl2w::TEXT_SIZE_24;
    style.fontColor = Colors::Black;
    break;
  case BaseFontConfig::MODAL_BUTTON:
    style.fontFamily = FontFamily::TITLE;
    style.fontSize = sdl2w::TEXT_SIZE_20;
    style.fontColor = Colors::White;
    break;
  }
}

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
  std::vector<std::unique_ptr<UiElement>> children;
  std::optional<StateInterface*> stateInterface;
  BaseStyle style;
  std::string id;
  std::vector<std::unique_ptr<UiEventObserver>> eventObservers;
  bool shouldPropagateEventsToChildren = true;

public:
  bool isHovered = false;
  bool isClicked = false;
  // Constructor
  UiElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  virtual ~UiElement() = default;

  virtual UiElement* getChildById(const std::string& id);
  virtual void removeChildById(const std::string& id);

  // Style methods
  virtual void setStyle(const BaseStyle& _style);
  virtual BaseStyle& getStyle();
  virtual const BaseStyle& getStyle() const;
  virtual const std::pair<int, int> getDims() const;

  // Id methods
  virtual void setId(const std::string& _id);
  virtual const std::string& getId() const;

  // Children methods
  virtual std::vector<std::unique_ptr<UiElement>>& getChildren();
  virtual const std::vector<std::unique_ptr<UiElement>>& getChildren() const;
  virtual void removeChildAtIndex(size_t index);
  virtual void addChild(UiElement* child);

  // Event handlers
  virtual bool checkMouseDownEvent(int mouseX,
                                   int mouseY,
                                   int button,
                                   std::vector<UiElement*> additionalElements = {});
  virtual bool checkMouseUpEvent(int mouseX,
                                 int mouseY,
                                 int button,
                                 std::vector<UiElement*> additionalElements = {});
  virtual bool checkHoverEvent(int mouseX,
                               int mouseY,
                               std::vector<UiElement*> additionalElements = {});
  virtual bool checkMouseWheelEvent(int mouseX,
                                    int mouseY,
                                    int delta,
                                    std::vector<UiElement*> additionalElements = {});
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

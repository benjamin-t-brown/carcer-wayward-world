#pragma once

#include "../lib/sdl2w/Defines.h"
#include "../lib/sdl2w/Window.h"
#include <SDL2/SDL_pixels.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
enum class FontFamily { PARAGRAPH, H1, H2, H3 };

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
  FontFamily fontFamily = FontFamily::PARAGRAPH;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = SDL_Color{255, 255, 255};
  TextAlign textAlign = TextAlign::LEFT_TOP;

  // List
  int lineHeight = 0;
  int lineSpacing = 0;
  SDL_Color lineBackgroundColor = SDL_Color{255, 255, 255, 0};
};

// Main UiElement base class
class UiElement {
protected:
  sdl2w::Window* window;
  UiElement* parent;
  std::vector<std::unique_ptr<UiElement>> children;
  std::optional<StateInterface*> stateInterface;
  BaseStyle style;
  std::string id;

public:
  // Constructor
  UiElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  virtual ~UiElement() = default;

  // State interface methods
  virtual void setStateInterface(StateInterface* _stateInterface);
  virtual void dispatchAction(const std::string& action, void* payload = nullptr);
  virtual void dispatchUiAction(const std::string& action, void* payload = nullptr);

  // Entity and template getters
  virtual UiElement* getEntityById(const std::string& id);
  virtual UiElement* getTemplateById(const std::string& id);

  // Style methods
  virtual void setStyle(const BaseStyle& _style);
  virtual BaseStyle& getStyle();
  virtual const BaseStyle& getStyle() const;

  // Id methods
  virtual void setId(const std::string& _id);
  virtual const std::string& getId() const;

  // Children methods
  virtual std::vector<std::unique_ptr<UiElement>>& getChildren();
  virtual const std::vector<std::unique_ptr<UiElement>>& getChildren() const;
  virtual void removeChildAtIndex(size_t index);

  // Event handlers
  virtual bool checkClickEvent(int mouseX, int mouseY);
  virtual bool checkHoverEvent(int mouseX, int mouseY);
  virtual void checkResizeEvent(int width, int height);

  // Build and render
  virtual void build();
  virtual void render();

  // Getters for window and parent
  sdl2w::Window* getWindow() const { return window; }
  UiElement* getParent() const { return parent; }
};

} // namespace ui


#pragma once

#include "../UiElement.h"
#include "types/WorldActions.h"
#include <SDL2/SDL_pixels.h>
#include <memory>
#include <string>

namespace ui {

// InGameLayout layout properties
struct InGameLayoutProps {
  std::vector<types::WorldActionType> worldActionTypes;
};

// InGameLayout layout - renders an in-game layout with background, border, title,
// subtitle, and children Uses Position, Size, Scale from BaseStyle
class InGameLayout : public UiElement {
private:
  InGameLayoutProps props;

public:
  InGameLayout(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameLayout() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const InGameLayoutProps& _props);
  InGameLayoutProps& getProps();
  const InGameLayoutProps& getProps() const;

  // Helper methods for child management
  UiElement* getChildById(const std::string& id);
  void removeChildById(const std::string& id);

  void setTitleElement(std::unique_ptr<UiElement> _titleElement);
  UiElement* getTitleElement();
  void setSubtitleElement(std::unique_ptr<UiElement> _subtitleElement);
  UiElement* getSubtitleElement();

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

#pragma once

#include "../UiElement.h"
#include "ui/elements/Quad.h"
#include <SDL2/SDL_pixels.h>

namespace ui {

// VerticalList-specific properties
struct VerticalListProps {
  int lineHeight = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// VerticalList component - renders an opinionated list of UiElements
// Uses Position, Size, Scale from BaseStyle
// Children are managed through base UiElement class
class VerticalList : public UiElement {
private:
  VerticalListProps props;
  int selectedIndex = -1; // -1 means no selection

  ui::Quad* getQuad();
  const ui::Quad* getQuad() const;

public:
  VerticalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~VerticalList() override = default;

  // Setters and getters for VerticalList-specific properties
  void setProps(const VerticalListProps& _props);
  VerticalListProps& getProps();
  const VerticalListProps& getProps() const;

  // Selection methods
  void setSelectedIndex(int index);
  int getSelectedIndex() const;
  void clearSelection();

  void addListItem(std::unique_ptr<UiElement> item);
  void addListItems(const std::vector<UiElement*>& items);
  void removeListItemAtIndex(size_t index);

  const std::pair<int, int> getDims() const override;

  // Override methods
  void build() override;
  void render(int dt) override;
};

} // namespace ui

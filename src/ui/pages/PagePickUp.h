#pragma once

#include "ui/UiElement.h"
#include <string>
#include <vector>

namespace ui {

// PagePickUp-specific properties
struct PagePickUpProps {
  std::vector<std::string> itemNames;
};

// PagePickUp - renders the pickup page with ModalStandard layout containing
// a scrollable list of items that can be picked up Uses Position, Size, Scale
// from BaseStyle
class PagePickUp : public UiElement {
private:
  PagePickUpProps props;

public:
  PagePickUp(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PagePickUp() override = default;

  // Setters and getters for page-specific properties
  void setProps(const PagePickUpProps& _props);
  PagePickUpProps& getProps();
  const PagePickUpProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui


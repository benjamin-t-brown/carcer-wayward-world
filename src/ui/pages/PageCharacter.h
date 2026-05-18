#pragma once

#include "ui/UiElement.h"

namespace ui {

struct PageCharacterProps {};

// PageCharacter - renders the character page with ModalStandard layout.
class PageCharacter : public UiElement {
private:
  PageCharacterProps props;

public:
  PageCharacter(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageCharacter() override = default;

  void setProps(const PageCharacterProps& _props);
  PageCharacterProps& getProps();
  const PageCharacterProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

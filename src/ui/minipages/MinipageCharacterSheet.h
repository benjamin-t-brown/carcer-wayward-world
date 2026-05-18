#pragma once

#include "ui/UiElement.h"

namespace ui {

struct MinipageCharacterSheetProps {};

// MinipageCharacterSheet - renders a small character sheet shell using ModalSmall.
class MinipageCharacterSheet : public UiElement {
private:
  MinipageCharacterSheetProps props;

public:
  MinipageCharacterSheet(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageCharacterSheet() override = default;

  void setProps(const MinipageCharacterSheetProps& _props);
  MinipageCharacterSheetProps& getProps();
  const MinipageCharacterSheetProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

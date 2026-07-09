#pragma once

#include "ui/UiElement.h"

namespace ui {

struct PageMagicSetupProps {
  int width = 0;
  int height = 0;
};

// PageMagicSetup - renders the magic page with ModalStandard layout.
class PageMagicSetup : public UiElement {
private:
  PageMagicSetupProps props;

public:
  PageMagicSetup(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageMagicSetup() override = default;

  void setProps(const PageMagicSetupProps& _props);
  PageMagicSetupProps& getProps();
  const PageMagicSetupProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

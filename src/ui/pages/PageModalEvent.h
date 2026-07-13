#pragma once

#include "ui/UiElement.h"
#include "ui/pages/PageTalkChoice.h"

namespace ui {

struct PageModalEventProps {
  int width = 500;
  int height = 400;
  bmin::String title;
  bmin::DynArray<PageTalkChoiceItem> choices;
  bmin::DynArray<TextBlock> textBlocks;
  bool showContinueButton = false;
};

// Centered small-modal page for MODAL special events (ModalSmall layout).
class PageModalEvent : public UiElement {
private:
  PageModalEventProps props;

public:
  PageModalEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageModalEvent() override = default;

  void setProps(const PageModalEventProps& _props);
  PageModalEventProps& getProps();
  const PageModalEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

#pragma once

#include "model/SpecialEvents.h"
#include "ui/UiElement.h"

namespace ui {

struct PageTalkChoiceProps {
  model::GameEvent gameEvent;
  std::string portraitName;
  int choiceAreaHeight = 100;
};

class PageTalkChoice : public UiElement {
private:
  PageTalkChoiceProps props;
  const int SEP_BORDER_HEIGHT = 10;

public:
  PageTalkChoice(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageTalkChoice() override = default;

  // Setters and getters for page-specific properties
  void setProps(const PageTalkChoiceProps& _props);
  PageTalkChoiceProps& getProps();
  const PageTalkChoiceProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

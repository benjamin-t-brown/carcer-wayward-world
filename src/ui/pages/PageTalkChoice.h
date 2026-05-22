#pragma once

#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"

namespace ui {

struct PageTalkChoiceItem {
  std::string nextId;
  std::string text;
  std::string prefixText;
};
struct PageTalkChoiceProps {
  std::string title;
  std::string portraitSpriteName;
  int choiceAreaHeight = 100;
  std::vector<PageTalkChoiceItem> choices;
  std::vector<TextBlock> textBlocks;
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

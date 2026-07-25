#pragma once

#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace ui {

struct PageTalkChoiceItem {
  bmin::String nextId;
  bmin::String text;
  bmin::String prefixText;
  bool previouslyChosen = false;
};
struct PageTalkChoiceProps {
  int width = 0;
  int height = 0;
  bmin::String title;
  bmin::String portraitSpriteName;
  // Passed through to ModalStandard; scales headerHeight + portrait together.
  float portraitScale = 1.f;
  int choiceAreaHeight = 100;
  bmin::DynArray<PageTalkChoiceItem> choices;
  bmin::DynArray<TextBlock> textBlocks;
  // Blocks [pinFromBlockIndex..) are the latest dialogue; scrolled to the top of the
  // text viewport with dynamic bottom padding so older history stays above.
  int pinFromBlockIndex = 0;
  // Multiplier for blank lines from `\n\n` in dialogue paragraphs (see TextParagraph).
  float blankLineHeightScale = 0.35f;
  // Multiplier for content line box height in dialogue paragraphs (see TextParagraph).
  float lineHeightScale = 0.85f;
};

class PageTalkChoice : public UiElement {
private:
  PageTalkChoiceProps props;
  const int SEP_BORDER_HEIGHT = 10;

  // Split dialogue on "..." boundaries: outside quotes → outsideColor (narrative);
  // inside quotes → Charcoal (spoken dialogue), unless the source already set a fontColor.
  static bmin::DynArray<TextBlock>
  colorizeDialogueByQuotes(const bmin::DynArray<TextBlock>& blocks,
                           SDL_Color outsideColor);

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

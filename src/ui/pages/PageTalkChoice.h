#pragma once

#include "ui/KeyboardHeldScroll.h"
#include "ui/KeyboardPressFlash.h"
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include <string_view>

namespace ui {

class ButtonTextWrap;
class SectionScrollable;

struct PageTalkChoiceItem {
  bmin::String nextId;
  bmin::String text;
  bmin::String prefixText;
  bool previouslyChosen = false;
  bool isContinue = false;
};
struct PageTalkChoiceProps {
  int width = 0;
  int height = 0;
  bmin::String title;
  bmin::String portraitSpriteName;
  // Passed through to ModalStandard; scales headerHeight + portrait together.
  float portraitScale = 1.f;
  int choiceAreaHeight = 250;
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
  KeyboardHeldScroll keyboardScroll;
  KeyboardPressFlash keyboardFlash;
  const int SEP_BORDER_HEIGHT = 10;

  // Split dialogue on "..." boundaries: outside quotes → outsideColor (narrative);
  // inside quotes → Charcoal (spoken dialogue), unless the source already set a fontColor.
  static bmin::DynArray<TextBlock>
  colorizeDialogueByQuotes(const bmin::DynArray<TextBlock>& blocks,
                           SDL_Color outsideColor);

  void setupKeyboardScroll();
  int findContinueChoiceIndex() const;
  void beginKeyboardContinuePress();
  void beginKeyboardChoicePress(int choiceIndex);
  void enqueueSelectChoice(int choiceIndex);

public:
  static constexpr int CHOICE_AREA_HEIGHT = 250;

  PageTalkChoice(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageTalkChoice() override = default;

  // Setters and getters for page-specific properties
  void setProps(const PageTalkChoiceProps& _props);
  PageTalkChoiceProps& getProps();
  const PageTalkChoiceProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  ButtonTextWrap* choiceButton(int i);
  SectionScrollable* textSection();
  SectionScrollable* choiceSection();

  void onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void updateKeyboardChrome(int deltaTime);
  void stopKeyboardChrome();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

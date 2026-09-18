#pragma once

#include "ui/KeyboardPressFlash.h"
#include "ui/UiElement.h"
#include "ui/pages/PageTalkChoice.h"
#include <string_view>

namespace ui {

class ButtonModal;
class ButtonTextWrap;

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
  KeyboardPressFlash keyboardFlash;

  void beginKeyboardContinuePress();
  void beginKeyboardChoicePress(int choiceIndex);
  void enqueueSelectChoice(int choiceIndex);
  void enqueueContinue();

public:
  PageModalEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageModalEvent() override = default;

  void setProps(const PageModalEventProps& _props);
  PageModalEventProps& getProps();
  const PageModalEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  ButtonTextWrap* choiceButton(int i);
  ButtonModal* continueButton();

  void onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void updateKeyboardChrome(int deltaTime);
  void stopKeyboardChrome();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

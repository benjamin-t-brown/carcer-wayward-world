#pragma once

#include "model/templates/UtilityTypes.h"
#include "ui/KeyboardPressFlash.h"
#include "ui/UiElement.h"
#include "ui/pages/PageTalkChoice.h"
#include <string_view>

namespace ui {

class ButtonGroup;
class ButtonModal;
class SectionScrollable;

struct PageModalEventProps {
  int width = 500;
  int height = 400;
  bmin::String title;
  bmin::DynArray<PageTalkChoiceItem> choices;
  bmin::DynArray<TextBlock> textBlocks;
  bool showContinueButton = false;
};

// Centered small-modal page for MODAL special events (ModalSmall layout).
// Choices appear as footer modal buttons (replacing Okay), not in-scroll dialogue rows.
class PageModalEvent : public UiElement {
private:
  PageModalEventProps props;
  KeyboardPressFlash keyboardFlash;

  enum class FooterMode { Continue, ShowMore, Choices, Inert };
  FooterMode footerMode = FooterMode::Inert;
  // Set by Show More; applied in updateKeyboardChrome so retargetFooter cannot
  // destroy the footer button while its onClick observer is still running.
  bool footerNeedsSync = false;

  // Linear Show More scroll over kShowMoreScrollMs.
  static constexpr int kShowMoreScrollMs = 100;
  bool showMoreAnimating = false;
  int showMoreScrollFrom = 0;
  int showMoreScrollTo = 0;
  model::TimerStruct showMoreTimer{kShowMoreScrollMs};

  void beginKeyboardContinuePress();
  void beginKeyboardChoicePress(int choiceIndex);
  void enqueueSelectChoice(int choiceIndex);
  void enqueueContinue();
  void performShowMore();
  void updateShowMoreScroll(int deltaTime);

  ButtonGroup* footerButtonGroup();
  // Pixels of real text ink below the fold (excludes empty bottom padding).
  int clippedTextPx();
  bool isContentClipped();
  FooterMode computeFooterMode();
  void retargetFooter(FooterMode mode);
  void styleShowMoreButton();
  void syncFooter(bool force);
  void renderShowMoreCue();

  // Ignore sub-line overflow from padding / rounding — must have real unread text.
  static constexpr int kMinShowMorePx = 12;

  friend class PageModalEventShowMoreObserver;

public:
  PageModalEvent(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageModalEvent() override = default;

  void setProps(const PageModalEventProps& _props);
  PageModalEventProps& getProps();
  const PageModalEventProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  ButtonModal* choiceButton(int i);
  ButtonModal* continueButton();
  SectionScrollable* textSection();

  void onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void updateKeyboardChrome(int deltaTime);
  void stopKeyboardChrome();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

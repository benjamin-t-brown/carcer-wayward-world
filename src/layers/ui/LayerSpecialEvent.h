#pragma once

#include "layers/Layer.h"
#include "lib/bmin/Map.h"
#include "model/templates/SpecialEvents.h"
#include "runner/SpecialEventRunner.h"
#include "ui/KeyboardHeldScroll.h"
#include "ui/elements/TextLine.h"
#include <optional>
#include <string_view>

namespace ui {
class ButtonModal;
class ButtonTextWrap;
class SectionScrollable;
}

namespace layers {

class LayerSpecialEvent : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_special_event";

  LayerSpecialEvent(sdl2w::Window* _window,
                    const model::GameEvent& gameEvent,
                    const bmin::Map<bmin::String, model::GameEvent>& gameEvents,
                    const bmin::Map<bmin::String, bmin::String>& initialStorage = {});
  ~LayerSpecialEvent() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void onChoiceSelected(int choiceIndex);
  void onContinue();
  void syncUi();

private:
  runner::SpecialEventRunner runner;
  runner::SpecialEventRunnerInterface runnerInterface;
  bmin::DynArray<ui::TextBlock> talkHistory;
  ui::KeyboardHeldScroll talkKeyboardScroll;
  bool eventFinished = false;
  bool needsSyncUi = false;
  int continuePressRemainingMs = 0;
  int choicePressRemainingMs = 0;
  std::optional<int> pendingChoiceIndex;

  void appendCurrentTalkTextToHistory();
  void appendTalkChoiceToHistory(int choiceIndex);
  void attachChoiceObservers();
  void attachModalContinueObserver();
  ui::ButtonModal* findModalContinueButton();
  ui::ButtonTextWrap* findChoiceButton(int choiceIndex);
  void beginKeyboardContinuePress();
  void beginKeyboardChoicePress(int choiceIndex);
  void closeLayer();
  void persistRunnerStorage();
  void setupTalkKeyboardScroll();
  ui::SectionScrollable* getTalkTextSection();
  ui::SectionScrollable* getTalkChoiceSection();
};

} // namespace layers

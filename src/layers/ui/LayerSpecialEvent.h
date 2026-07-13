#pragma once

#include "layers/Layer.h"
#include "lib/bmin/Map.h"
#include "model/templates/SpecialEvents.h"
#include "runner/SpecialEventRunner.h"
#include <string_view>

namespace ui {
class ButtonModal;
}

namespace layers {

class LayerSpecialEvent : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_special_event";

  LayerSpecialEvent(sdl2w::Window* _window,
                    const model::GameEvent& gameEvent,
                    const bmin::Map<bmin::String, model::GameEvent>& gameEvents);
  ~LayerSpecialEvent() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void update(int deltaTime) override;
  void onChoiceSelected(int choiceIndex);
  void onContinue();
  void syncUi();

private:
  runner::SpecialEventRunner runner;
  runner::SpecialEventRunnerInterface runnerInterface;
  bool eventFinished = false;
  bool needsSyncUi = false;
  int continuePressRemainingMs = 0;

  void attachChoiceObservers();
  void attachModalContinueObserver();
  ui::ButtonModal* findModalContinueButton();
  void beginKeyboardContinuePress();
  void closeLayer();
};

} // namespace layers

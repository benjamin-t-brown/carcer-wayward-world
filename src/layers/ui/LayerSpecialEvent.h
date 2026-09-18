#pragma once

#include "layers/UiLayer.h"
#include "bmin/Map.h"
#include "game/SpecialEventPresenter.h"
#include "model/templates/SpecialEvents.hpp"
#include "in3/SpecialEventRunner.h"
#include <string_view>

namespace ui {
class PageModalEvent;
class PageTalkChoice;
}

namespace layers {

class LayerSpecialEvent : public UiLayer {
private:
  in3::SpecialEventRunner runner;
  in3::SpecialEventRunnerInterface runnerInterface;
  bmin::DynArray<game::SpecialEventTranscriptEntry> talkHistory;
  bool needsSyncUi = false;

  bool isTalkEvent() const;
  ui::PageTalkChoice* talkPage();
  ui::PageModalEvent* modalPage();
  void closeLayer();
  void persistRunnerStorage();
  void stopEventPageKeyboardChrome();
  void updateEventPageKeyboardChrome(int deltaTime);

public:
  constexpr static std::string_view LAYER_ID = "layer_special_event";

  LayerSpecialEvent(sdl2w::Window* _window,
                    const model::GameEvent& gameEvent,
                    const bmin::Map<bmin::String, model::GameEvent>& gameEvents,
                    const bmin::Map<bmin::String, bmin::String>& initialStorage = {});
  ~LayerSpecialEvent() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void onChoiceSelected(int choiceIndex);
  void onContinue();
  void syncUi();
  void update(int deltaTime) override;
};

} // namespace layers

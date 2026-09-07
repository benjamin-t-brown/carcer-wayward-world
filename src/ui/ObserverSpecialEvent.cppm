module;
#include <cstddef>
#include <cstdint>

export module carcer.ui.ObserverSpecialEvent;
import carcer.actions;
export import carcer.ui.core;
import carcer.state;
import sdl2w;
#include "macros.h"

export {

// Choice / continue clicks in a special event go through the action bus, not
// a back-pointer into layers::LayerSpecialEvent — UI observers don't reach
// into a concrete Layer. LayerSpecialEvent subscribes to react.
namespace ui {

class ObserverSpecialEventChoice : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  int choiceIndex;

public:
  explicit ObserverSpecialEventChoice(int _choiceIndex) : choiceIndex(_choiceIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::selectSpecialEventChoice(choiceIndex),
        0);
  }
};

class ObserverSpecialEventContinue : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
public:
  void onClick(int mouseX, int mouseY, int button) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::continueSpecialEvent(), 0);
  }
};

} // namespace ui

} // export

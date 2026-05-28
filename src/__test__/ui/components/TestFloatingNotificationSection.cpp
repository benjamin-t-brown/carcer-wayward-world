#include "../../setupTestUi.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "state/actions/ui/UiPushFloatingNotification.hpp"
#include "ui/UiElement.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/buttons/ButtonModal.h"
#include <memory>

namespace {

class PushNotificationObserver : public ui::UiEventObserver {
  state::StateManager* stateManager;
  int& counter;

public:
  PushNotificationObserver(state::StateManager* _stateManager, int& _counter)
      : stateManager(_stateManager), counter(_counter) {}

  void onClick(int, int, int) override {
    counter++;
    const auto type = static_cast<state::UiFloatingNotificationType>((counter - 1) % 3);
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiPushFloatingNotification(
            "Notification #" + std::to_string(counter), type),
        0);
  }
};

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start FloatingNotificationSection test" << LOG_ENDL;

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  std::vector<std::unique_ptr<ui::UiElement>> elements;
  int notificationCounter = 0;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto pushButton = new ui::ButtonModal(&window);
    pushButton->setId("pushNotificationButton");
    auto& buttonStyle = pushButton->getStyle();
    buttonStyle.x = 50;
    buttonStyle.y = 50;
    buttonStyle.width = 260;
    buttonStyle.height = 50;
    pushButton->setProps(ui::ButtonModalProps{
        .text = "Push Notification",
        .isSelected = false,
    });
    pushButton->addEventObserver(new PushNotificationObserver(&stateManager, notificationCounter));
    elements.push_back(std::unique_ptr<ui::UiElement>(pushButton));

    auto notificationSection = new ui::FloatingNotificationSection(&window);
    notificationSection->setId("floatingNotificationSection");
    elements.push_back(std::unique_ptr<ui::UiElement>(notificationSection));

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          for (auto& elem : elements) {
            elem->checkMouseDownEvent(x, y, button);
          }
        });
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          for (auto& elem : elements) {
            elem->checkMouseUpEvent(x, y, button);
          }
        });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    stateManager.update(window.getDeltaTime());

    auto& events = window.getEvents();
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(events.mouseX, events.mouseY);
        elem->render(window.getDeltaTime());
      }
    }

    return true;
  };

  setupTestUi(
      argc,
      argv,
      TestUiParams{800, 600, "FloatingNotificationSection Test"},
      _init,
      _updateRender,
      [&]() { elements.clear(); });

  LOG(INFO) << "End FloatingNotificationSection test" << LOG_ENDL;
  return 0;
}

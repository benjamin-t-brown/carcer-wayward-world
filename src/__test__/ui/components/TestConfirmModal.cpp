#include "../../setupTestUi.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/components/ConfirmModal.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include <memory>
#include "lib/Types.h"

namespace {

class TestConfirmModalObserver : public ui::UiEventObserver {
  String buttonId;

public:
  TestConfirmModalObserver(String _buttonId) : buttonId(_buttonId) {}

  void onClick(int x, int y, int button) override {
    LOG(INFO) << "ConfirmModal " << buttonId << " clicked at " << x << ", " << y
              << " button=" << button << LOG_ENDL;
  }
};

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start ConfirmModal test" << LOG_ENDL;

  DynArray<UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ConfirmModal test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    auto modal = new ui::ConfirmModal(&window);
    modal->setId("confirmModal");
    modal->setProps(ui::ConfirmModalProps{
        .title = "Drop item",
        .message = "Are you sure you wish to drop Iron Sword? This longer message "
                   "should wrap within the fixed modal width.",
        .confirmButtonLabel = "Yes",
        .cancelButtonLabel = "No",
    });

    auto& style = modal->getStyle();
    style.scale = 1.f;
    const auto [modalW, modalH] = modal->getDims();
    style.x = (windowWidth - modalW) / 2;
    style.y = (windowHeight - modalH) / 2;
    modal->build();

    auto* buttonGroup = modal->getButtonGroup();
    if (buttonGroup != nullptr) {
      buttonGroup->addObserverToButtonAtIndex(0, new TestConfirmModalObserver("cancel"));
      buttonGroup->addObserverToButtonAtIndex(1, new TestConfirmModalObserver("confirm"));
    } else {
      LOG(ERROR) << "ConfirmModal getButtonGroup() returned nullptr" << LOG_ENDL;
    }

    elements.pushBack(UniquePtr<ui::UiElement>(modal));

    auto& events = window.getEvents();
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                         [&](int x, int y, int button) {
                           for (auto& elem : elements) {
                             elem->checkMouseDownEvent(x, y, button);
                           }
                         });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      for (auto& elem : elements) {
        elem->checkMouseUpEvent(x, y, button);
      }
    });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& events = window.getEvents();
    for (auto& elem : elements) {
      elem->checkHoverEvent(events.mouseX, events.mouseY);
      elem->render(window.getDeltaTime());
    }

    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{640, 480, "ConfirmModal Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });

  LOG(INFO) << "End ConfirmModal test" << LOG_ENDL;
  return 0;
}

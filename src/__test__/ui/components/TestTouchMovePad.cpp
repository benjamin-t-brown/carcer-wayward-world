#include "../../setupTestUi.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/TouchMovePad.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start TouchMovePad test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "TouchMovePad test initialized" << LOG_ENDL;

    auto movePad = new ui::TouchMovePad(&window);
    movePad->setId("touchMovePad");
    movePad->setPos(80, 60);
    movePad->setScale(1.f);
    movePad->setProps(ui::TouchMovePadProps{});
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(movePad));

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
              TestUiParams{320, 280, "TouchMovePad Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End TouchMovePad test" << LOG_ENDL;
  return 0;
}

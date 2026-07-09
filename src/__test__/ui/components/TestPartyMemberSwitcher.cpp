#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/PartyMemberSwitcher.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start PartyMemberSwitcher test" << LOG_ENDL;
  srand(time(NULL));

  DynArray<UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PartyMemberSwitcher test initialized" << LOG_ENDL;

    {
      auto switcher = new ui::PartyMemberSwitcher(&window);
      auto& style = switcher->getStyle();
      style.x = 200;
      style.y = 200;
      style.scale = 2.0f;
      switcher->setProps(ui::PartyMemberSwitcherProps{
          .spriteName = "actors0_0",
          .partyMemberIndex = 0,
      });
      elements.pushBack(UniquePtr<ui::UiElement>(switcher));
    }

    {
      auto switcher = new ui::PartyMemberSwitcher(&window);
      auto& style = switcher->getStyle();
      style.x = 200;
      style.y = 320;
      style.scale = 1.0f;
      switcher->setProps(ui::PartyMemberSwitcherProps{
          .spriteName = "actors0_6",
          .partyMemberIndex = 2,
      });
      elements.pushBack(UniquePtr<ui::UiElement>(switcher));
    }

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
    auto& draw = window.getDraw();
    draw.clearScreen();

    auto& events = window.getEvents();
    for (auto& elem : elements) {
      elem->checkHoverEvent(events.mouseX, events.mouseY);
      elem->render(window.getDeltaTime());
    }
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "PartyMemberSwitcher Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End PartyMemberSwitcher test" << LOG_ENDL;
  return 0;
}

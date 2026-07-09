#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/InGameTitleBar.h"
#include <memory>

class ToggleApObserver : public ui::UiEventObserver {
  ui::InGameTitleBar* titleBar;

public:
  explicit ToggleApObserver(ui::InGameTitleBar* _titleBar) : titleBar(_titleBar) {}
  ~ToggleApObserver() override = default;

  void onClick(int x, int y, int button) override {
    auto newProps = titleBar->getProps();
    newProps.showAp = !newProps.showAp;
    titleBar->setProps(newProps);
  }
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start InGameTitleBar test" << LOG_ENDL;
  srand(time(NULL));

  DynArray<UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "InGameTitleBar test initialized" << LOG_ENDL;

    auto titleBar = new ui::InGameTitleBar(&window);
    titleBar->setId("titleBar");
    auto& style = titleBar->getStyle();
    style.x = 16;
    style.y = 24;
    style.width = 768;
    style.height = 36;
    style.scale = 1.f;
    titleBar->setProps(ui::InGameTitleBarProps{
        .title = "Boramark",
        .day = 12,
        .food = 8,
        .ap = 4,
    });
    elements.pushBack(UniquePtr<ui::UiElement>(titleBar));

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
              TestUiParams{800, 120, "InGameTitleBar Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End InGameTitleBar test" << LOG_ENDL;
  return 0;
}

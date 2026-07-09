#include "../../setupTestUi.h"
#include "layers/Layer.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/popups/PopupPickupItem.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

class TestLayer : public layers::Layer {
public:
  TestLayer(sdl2w::Window* _window) : layers::Layer(_window, "testPickupPopupLayer") {}
};

int main(int argc, char** argv) {
  LOG(INFO) << "Start PopupPickupItem test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;
  bmin::UniquePtr<TestLayer> testLayer;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PopupPickupItem test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    testLayer = bmin::makeUnique<TestLayer>(&window);

    auto scale = 1.f;
    auto orientation = ui::PopupOrientation::WIDE;
    auto popupPickupItem = new ui::PopupPickupItem(&window, testLayer.get(), orientation);
    popupPickupItem->setId("popupPickupItem");
    popupPickupItem->setProps(ui::PopupPickupItemProps{
        .spriteName = "ui_item_icons_0",
        .label = "Test Item 1",
        .description = "This is a test item description. It should gracefully wrap in "
                       "the tooltip and look pretty good in the process.",
        .weight = 10,
        .value = 100,
        .orientation = orientation,
    });
    auto [popupW, popupH] = popupPickupItem->getDims();
    popupPickupItem->setPos((windowWidth - popupW * scale) / 2,
                            (windowHeight - popupH * scale) / 2);
    popupPickupItem->setScale(scale);
    popupPickupItem->build();
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(popupPickupItem));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          //
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          for (auto& elem : elements) {
            elem->checkMouseDownEvent(x, y, button);
          }
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          //
          for (auto& elem : elements) {
            elem->checkMouseUpEvent(x, y, button);
          }
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
        [&](int x, int y, int delta) {
          for (auto& elem : elements) {
            elem->checkMouseWheelEvent(x, y, delta);
          }
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& elem : elements) {
      elem->checkHoverEvent(window.getEvents().mouseX, window.getEvents().mouseY);
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();

    for (auto& elem : elements) {
      elem->render(window.getDeltaTime());
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "PopupPickupItem Test"},
              _init,
              _updateRender,
              [&]() {
                elements.clear();
                testLayer.reset();
              });
  LOG(INFO) << "End PopupPickupItem test" << LOG_ENDL;
  return 0;
}

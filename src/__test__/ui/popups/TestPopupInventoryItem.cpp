#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/popups/PopupInventoryItem.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start PopupInventoryItem test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PopupInventoryItem test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    auto scale = 1.f;
    auto orientation = ui::PopupOrientation::WIDE;
    auto popupInventoryItem = new ui::PopupInventoryItem(&window, nullptr, orientation);
    popupInventoryItem->setId("minipagePickUp");
    auto& style = popupInventoryItem->getStyle();
    style.x = (windowWidth - style.width * scale) / 2;
    style.y = (windowHeight - style.height * scale) / 2;
    style.scale = scale;
    popupInventoryItem->setProps(ui::PopupInventoryItemProps{
        .item =
            {
                .id = model::createRandomId(),
                .itemName = "TestItem1",
                .quantity = 1,
            },
        .label = "Test Item 1",
        .description = "This is a test item description. It should gracefully wrap in "
                       "the tooltip and look pretty good in the process.",
        .spriteName = "ui_item_icons_0",
        .weight = 10,
        .value = 100,
        .usable = true,
        .equippable = true,
        .orientation = orientation,
    });
    elements.push_back(std::unique_ptr<ui::UiElement>(popupInventoryItem));

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

    // Render all elements
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
              TestUiParams{800, 600, "PopupInventoryItem Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End PopupInventoryItem test" << LOG_ENDL;
  return 0;
}

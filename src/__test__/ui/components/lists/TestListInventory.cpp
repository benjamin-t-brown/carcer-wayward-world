#include "../../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/components/lists/ListInventory.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start ListInventory test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ListInventory test initialized" << LOG_ENDL;

    auto listInventory = new ui::ListInventory(&window, nullptr);
    listInventory->setId("listInventory");
    ui::BaseStyle style = listInventory->getStyle();
    style.width = 500;
    style.x = 100;
    style.y = 50;
    listInventory->setStyle(style);

    listInventory->setProps({
        .items =
            {
                {
                    .itemName = "item1",
                    .itemLabel = "Item 1",
                    .itemSprite = "ui_item_icons_0",
                },
                {
                    .itemName = "item2",
                    .itemLabel = "Item 2",
                    .itemSprite = "ui_item_icons_1",
                },
                {
                    .itemName = "item3",
                    .itemLabel = "Item 3",
                    .itemSprite = "ui_item_icons_2",
                },
                {
                    .itemName = "item4",
                    .itemLabel = "Item 4",
                    .itemSprite = "ui_item_icons_3",
                },
                {
                    .itemName = "item5",
                    .itemLabel = "Item 5",
                    .itemSprite = "ui_item_icons_4",
                },
                {
                    .itemName = "item6",
                    .itemLabel = "Item 6",
                    .itemSprite = "ui_item_icons_5",
                },
                {
                    .itemName = "item7",
                    .itemLabel = "Item 7",
                    .itemSprite = "ui_item_icons_6",
                },
            },
    });

    elements.push_back(std::unique_ptr<ui::UiElement>(listInventory));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          //
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          for (auto& element : elements) {
            element->checkMouseDownEvent(x, y, button);
          }
        });
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_UP,
        [&](int x, int y, int button) {
          //
          for (auto& element : elements) {
            element->checkMouseUpEvent(x, y, button);
          }
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& element : elements) {
      element->checkHoverEvent(window.getEvents().mouseX, window.getEvents().mouseY);
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();

    // Render all elements
    for (auto& element : elements) {
      element->render(window.getDeltaTime());
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "ListInventory Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End ListInventory test" << LOG_ENDL;
  return 0;
}

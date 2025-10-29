#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "types/Items.h"
#include "ui/UiElement.h"
#include "ui/lists/ListInventory.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start ListInventory test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  db::Database database;
  database.load();

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ListInventory test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    // Create ListInventory component
    auto listInventory = std::make_unique<ui::ListInventory>(&window, nullptr, &database);
    listInventory->setId("listInventory");
    ui::BaseStyle style = listInventory->getStyle();
    style.width = 500;
    style.height = 400;
    style.x = 1;
    style.y = 1;
    listInventory->setStyle(style);

    ui::ListInventoryProps listInventoryProps;
    listInventoryProps.itemNames = {
        "PotionHealing",
        "PotionHealing",
        "PotionHealing",
    };
    listInventory->setProps(listInventoryProps);

    elements.push_back(std::move(listInventory));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
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
          for (auto& elem : elements) {
            elem->checkMouseUpEvent(x, y, button);
          }
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    // Check hover events for all elements
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
      }
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
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

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "ListInventory Test"}, _init, _updateRender);
  LOG(INFO) << "End ListInventory test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

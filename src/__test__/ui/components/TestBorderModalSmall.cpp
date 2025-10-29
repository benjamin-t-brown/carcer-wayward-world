#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/BorderModalSmall.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start BorderModalSmall test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "BorderModalSmall test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();
    // Create BorderModalSmall component
    auto borderModal = std::make_unique<ui::BorderModalSmall>(&window);
    ui::BaseStyle style;
    style.width = 500;
    style.height = windowHeight - 50;
    style.x = (windowWidth - style.width) / 2;
    style.y = (windowHeight - style.height) / 2;
    style.scale = 1.0f;
    borderModal->setStyle(style);

    elements.push_back(std::move(borderModal));

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

    // Check hover events for all buttons
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
      argc, argv, TestUiParams{800, 600, "BorderModalSmall Test"}, _init, _updateRender);
  LOG(INFO) << "End BorderModalSmall test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

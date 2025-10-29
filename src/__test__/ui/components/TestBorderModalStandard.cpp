#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/BorderModalStandard.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start BorderModalStandard test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "BorderModalStandard test initialized" << LOG_ENDL;

    // Create BorderModalStandard component
    auto borderModal = std::make_unique<ui::BorderModalStandard>(&window);
    ui::BaseStyle style;
    style.x = 0;
    style.y = 0;
    style.width = window.getDims().first;
    style.height = window.getDims().second;
    style.scale = 1.0f;
    borderModal->setStyle(style);

    elements.push_back(std::move(borderModal));
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    // Update logic if needed
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

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "BorderModalStandard Test"},
              _init,
              _updateRender);
  LOG(INFO) << "End BorderModalStandard test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

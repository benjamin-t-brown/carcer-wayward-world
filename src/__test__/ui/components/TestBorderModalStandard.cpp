#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
import carcer.ui.widgets;
import sdl2w;
import bmin.string_interop;
#include "macros.h"
#include "../../setupTestUi.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start BorderModalStandard test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "BorderModalStandard test initialized" << LOG_ENDL;

    // Create BorderModalStandard component
    auto borderModal = bmin::makeUnique<ui::BorderModalStandard>(&window);
    borderModal->setPos(0, 0);
    borderModal->setScale(1.0f);
    borderModal->setProps(ui::BorderModalSmallProps{
        .width = window.getDims().first,
        .height = window.getDims().second,
    });

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(borderModal.release()));
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
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End BorderModalStandard test" << LOG_ENDL;
  return 0;
}

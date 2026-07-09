#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/buttons/ButtonClose.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start OutsetRectangle test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "OutsetRectangle test initialized" << LOG_ENDL;

    {
      auto outsetRect1 = new ui::OutsetRectangle(&window);
      outsetRect1->setPos(10, 10);
      outsetRect1->setScale(1.0f);
      outsetRect1->setProps(ui::OutsetRectangleProps{
          .width = 200,
          .height = 200,
      });

      elements.pushBack(bmin::UniquePtr<ui::UiElement>(outsetRect1));
    }

    {
      auto rect = new ui::OutsetRectangle(&window);
      rect->setPos(250, 50);
      rect->setScale(4.f);
      rect->setProps(ui::OutsetRectangleProps{
          .width = 100,
          .height = 100,
          .color = ui::Colors::White,
          .colorTopRight = ui::Colors::LightBlue,
          .colorBottomLeft = ui::Colors::DarkBlue,
          .borderSize = 4,
      });

      elements.pushBack(bmin::UniquePtr<ui::UiElement>(rect));
    }
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
              TestUiParams{800, 600, "OutsetRectangle Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End OutsetRectangle test" << LOG_ENDL;
  return 0;
}

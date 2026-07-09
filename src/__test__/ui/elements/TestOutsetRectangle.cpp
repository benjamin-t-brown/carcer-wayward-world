#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
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
      auto& style1 = outsetRect1->getStyle();
      style1.x = 10;
      style1.y = 10;
      style1.width = 200;
      style1.height = 200;
      style1.scale = 1.0f;

      ui::OutsetRectangleProps props1;
      // props1.color = ui::Colors::White;
      // props1.colorTopRight = ui::Colors::LightBlue;
      // props1.colorBottomLeft = ui::Colors::Black;
      // props1.borderSize = 3;
      outsetRect1->setProps(props1);

      elements.pushBack(bmin::UniquePtr<ui::UiElement>(outsetRect1));
    }

    {
      auto rect = new ui::OutsetRectangle(&window);
      auto& style = rect->getStyle();
      style.x = 250;
      style.y = 50;
      style.width = 100;
      style.height = 100;
      style.scale = 4.f;

      ui::OutsetRectangleProps props;
      props.color = ui::Colors::White;
      props.colorTopRight = ui::Colors::LightBlue;
      props.colorBottomLeft = ui::Colors::DarkBlue;
      props.borderSize = 4;
      rect->setProps(props);

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

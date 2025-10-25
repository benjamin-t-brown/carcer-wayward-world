#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start OutsetRectangle test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "OutsetRectangle test initialized" << LOG_ENDL;

    // Create first outset rectangle with default colors
    auto outsetRect1 = std::make_unique<ui::OutsetRectangle>(&window);
    ui::BaseStyle style1;
    style1.x = 50;
    style1.y = 50;
    style1.width = 200;
    style1.height = 200;
    style1.scale = 1.0f;
    outsetRect1->setStyle(style1);

    ui::OutsetRectangleProps props1;
    // props1.color = ui::Colors::White;
    // props1.colorTopRight = ui::Colors::LightBlue;
    // props1.colorBottomLeft = ui::Colors::Black;
    // props1.borderSize = 3;
    outsetRect1->setProps(props1);

    elements.push_back(std::move(outsetRect1));

    // // Create second outset rectangle with custom RGBA colors
    // auto outsetRect2 = std::make_unique<ui::OutsetRectangle>(&window);
    // ui::BaseStyle style2;
    // style2.x = 300;
    // style2.y = 50;
    // style2.width = 200;
    // style2.height = 100;
    // style2.scale = 1.0f;
    // outsetRect2->setStyle(style2);

    // ui::OutsetRectangleProps props2;
    // props2.color = SDL_Color{255, 200, 200, 255};  // Light red
    // props2.colorTopRight = SDL_Color{255, 255, 255, 255};  // White
    // props2.colorBottomLeft = SDL_Color{100, 100, 100, 255};  // Dark grey
    // props2.borderSize = 5;
    // outsetRect2->setProps(props2);

    // elements.push_back(std::move(outsetRect2));

    // // Create third outset rectangle with different scale
    // auto outsetRect3 = std::make_unique<ui::OutsetRectangle>(&window);
    // ui::BaseStyle style3;
    // style3.x = 50;
    // style3.y = 200;
    // style3.width = 150;
    // style3.height = 80;
    // style3.scale = 1.5f;
    // outsetRect3->setStyle(style3);

    // ui::OutsetRectangleProps props3;
    // props3.color = SDL_Color{200, 255, 200, 255};  // Light green
    // props3.colorTopRight = ui::Colors::LightBlue;
    // props3.colorBottomLeft = ui::Colors::Black;
    // props3.borderSize = 2;
    // outsetRect3->setProps(props3);

    // elements.push_back(std::move(outsetRect3));

    // // Create fourth outset rectangle with no border
    // auto outsetRect4 = std::make_unique<ui::OutsetRectangle>(&window);
    // ui::BaseStyle style4;
    // style4.x = 300;
    // style4.y = 200;
    // style4.width = 200;
    // style4.height = 100;
    // style4.scale = 1.0f;
    // outsetRect4->setStyle(style4);

    // ui::OutsetRectangleProps props4;
    // props4.color = ui::Colors::Blue;
    // props4.colorTopRight = ui::Colors::LightBlue;
    // props4.colorBottomLeft = ui::Colors::Black;
    // props4.borderSize = 0;
    // outsetRect4->setProps(props4);

    // elements.push_back(std::move(outsetRect4));
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    // Update logic if needed
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.clearScreen();

    // Render all elements
    for (auto& element : elements) {
      element->render();
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "OutsetRectangle Test"}, _init, _updateRender);
  LOG(INFO) << "End OutsetRectangle test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

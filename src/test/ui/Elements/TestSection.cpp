#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/SectionDropShadow.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start SectionDropShadow test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "SectionDropShadow test initialized" << LOG_ENDL;

    // Create main section with drop shadow
    auto section = std::make_unique<ui::SectionDropShadow>(&window);
    section->setId("mainSection");
    ui::BaseStyle sectionStyle;
    sectionStyle.x = 100;
    sectionStyle.y = 100;
    sectionStyle.width = 400;
    sectionStyle.height = 300;
    section->setStyle(sectionStyle);

    ui::SectionDropShadowProps sectionProps;
    sectionProps.backgroundColor = ui::Colors::White;
    sectionProps.shadowColor = ui::Colors::Black;
    sectionProps.borderSize = 2;
    sectionProps.isSelected = false;
    section->setProps(sectionProps);
    elements.push_back(std::move(section));

    // Create a second section with different styling
    auto section2 = std::make_unique<ui::SectionDropShadow>(&window);
    section2->setId("secondarySection");
    ui::BaseStyle section2Style;
    section2Style.x = 550;
    section2Style.y = 150;
    section2Style.width = 200;
    section2Style.height = 200;
    section2->setStyle(section2Style);

    ui::SectionDropShadowProps section2Props;
    section2Props.backgroundColor = ui::Colors::ButtonModalGrey1;
    section2Props.shadowColor = ui::Colors::Black;
    section2Props.shadowOffsetX = -4;
    section2Props.shadowOffsetY = 4;
    section2Props.borderSize = 1;
    section2Props.isSelected = true; // Show selection state
    section2->setProps(section2Props);
    elements.push_back(std::move(section2));

    // Create a small section to show different sizes
    auto section3 = std::make_unique<ui::SectionDropShadow>(&window);
    section3->setId("smallSection");
    ui::BaseStyle section3Style;
    section3Style.x = 100;
    section3Style.y = 450;
    section3Style.width = 150;
    section3Style.height = 100;
    section3->setStyle(section3Style);

    ui::SectionDropShadowProps section3Props;
    section3Props.backgroundColor = ui::Colors::White;
    section3Props.shadowColor = ui::Colors::Black;
    section3Props.shadowOffsetX = -3;
    section3Props.shadowOffsetY = 3;
    section3Props.borderSize = 1;
    section3Props.isSelected = false;
    section3->setProps(section3Props);
    elements.push_back(std::move(section3));

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

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    window.getDraw().setBackgroundColor({70, 70, 70});
    // Get mouse position for hover events
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    // Check hover events for all sections
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
        elem->render();
      }
    }

    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "SectionDropShadow Test"}, _init, _updateRender);
  LOG(INFO) << "End SectionDropShadow test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

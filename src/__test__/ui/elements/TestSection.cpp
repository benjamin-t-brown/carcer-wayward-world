#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/components/borders/BorderDropShadow.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start BorderDropShadow test" << LOG_ENDL;
  srand(time(NULL));

  DynArray<UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "BorderDropShadow test initialized" << LOG_ENDL;

    auto section = makeUnique<ui::BorderDropShadow>(&window);
    section->setId("mainSection");
    auto& sectionStyle = section->getStyle();
    sectionStyle.x = 100;
    sectionStyle.y = 100;
    sectionStyle.width = 400;
    sectionStyle.height = 300;
    section->setProps(ui::BorderDropShadowProps{
        .backgroundColor = ui::Colors::White,
        .shadowColor = ui::Colors::Black,
        .borderSize = 2,
        .isSelected = false,
    });
    elements.pushBack(UniquePtr<ui::UiElement>(section.release()));

    auto section2 = makeUnique<ui::BorderDropShadow>(&window);
    section2->setId("secondarySection");
    auto& section2Style = section2->getStyle();
    section2Style.x = 550;
    section2Style.y = 150;
    section2Style.width = 200;
    section2Style.height = 200;
    section2->setProps(ui::BorderDropShadowProps{
        .backgroundColor = ui::Colors::ButtonModalGrey1,
        .shadowColor = ui::Colors::Black,
        .shadowOffsetX = -4,
        .shadowOffsetY = 4,
        .borderSize = 1,
        .isSelected = true,
    });
    elements.pushBack(UniquePtr<ui::UiElement>(section2.release()));

    auto section3 = makeUnique<ui::BorderDropShadow>(&window);
    section3->setId("smallSection");
    auto& section3Style = section3->getStyle();
    section3Style.x = 100;
    section3Style.y = 450;
    section3Style.width = 150;
    section3Style.height = 100;
    section3->setProps(ui::BorderDropShadowProps{
        .backgroundColor = ui::Colors::White,
        .shadowColor = ui::Colors::Black,
        .shadowOffsetX = -3,
        .shadowOffsetY = 3,
        .borderSize = 1,
        .isSelected = false,
    });
    elements.pushBack(UniquePtr<ui::UiElement>(section3.release()));

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button
                    << LOG_ENDL;
          for (auto& elem : elements) {
            elem->checkMouseDownEvent(x, y, button);
          }
        });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      for (auto& elem : elements) {
        elem->checkMouseUpEvent(x, y, button);
      }
    });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    window.getDraw().setBackgroundColor({70, 70, 70});
    auto& events = window.getEvents();
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(events.mouseX, events.mouseY);
        elem->render(window.getDeltaTime());
      }
    }
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "BorderDropShadow Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End BorderDropShadow test" << LOG_ENDL;
  return 0;
}

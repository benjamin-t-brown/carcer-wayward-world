#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start ModalStandard test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ModalStandard test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    // Create ModalStandard layout
    auto modalLayout = bmin::makeUnique<ui::ModalStandard>(&window);
    modalLayout->setPos(0, 0);
    // Set layout properties
    ui::ModalStandardProps props;
    props.width = windowWidth;
    props.height = windowHeight;
    props.contentBackgroundColor = ui::Colors::ModalStandardBackground;
    props.decorationSprite = "";
    modalLayout->setProps(props);

    // Create title and subtitle elements
    auto title = bmin::makeUnique<ui::TextLine>(&window);
    ui::TextFontProps titleFont;
    ui::setBaseFontConfig(titleFont, ui::BaseFontConfig::MODAL_TITLE);
    ui::TextLineProps titleProps;
    titleProps.fontFamily = titleFont.fontFamily;
    titleProps.fontSize = sdl2w::TEXT_SIZE_24;
    titleProps.fontColor = titleFont.fontColor;
    ui::TextBlock titleBlock;
    titleBlock.text = "Modal Title";
    titleProps.textBlocks.pushBack(titleBlock);
    title->setProps(titleProps);
    modalLayout->setTitleElement(title.release());

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(modalLayout.release()));

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
      argc, argv, TestUiParams{800, 600, "ModalStandard Test"}, _init, _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ModalStandard test" << LOG_ENDL;
  return 0;
}

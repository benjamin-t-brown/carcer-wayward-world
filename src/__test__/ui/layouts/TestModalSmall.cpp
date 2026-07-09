#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalSmall.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start ModalSmall test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ModalSmall test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    // Create ModalSmall layout
    auto modalSmall = new ui::ModalSmall(&window);
    const int modalWidth = 500;
    const int modalHeight = windowHeight - 50;
    modalSmall->setPos((windowWidth - modalWidth) / 2, (windowHeight - modalHeight) / 2);

    // Set layout properties
    ui::ModalSmallProps props;
    props.width = modalWidth;
    props.height = modalHeight;
    props.backgroundColor = ui::Colors::ModalStandardBackground;
    props.iconSprite = "";
    modalSmall->setProps(props);

    // Create title
    auto title = new ui::TextLine(&window, modalSmall);
    ui::TextFontProps titleFont;
    ui::setBaseFontConfig(titleFont, ui::BaseFontConfig::MODAL_TITLE);
    ui::TextLineProps titleProps;
    titleProps.fontFamily = titleFont.fontFamily;
    titleProps.fontSize = sdl2w::TEXT_SIZE_24;
    titleProps.fontColor = ui::Colors::Black;
    ui::TextBlock titleBlock;
    titleBlock.text = "Small Modal Title";
    titleProps.textBlocks.pushBack(titleBlock);
    title->setProps(titleProps);
    modalSmall->setTitleElement(title);

    // auto subtitle = bmin::makeUnique<ui::TextLine>(&window);
    // subtitle->setPos(0, 0);
    // subtitle->setProps(ui::TextLineProps{
    //     .textBlocks = {{.text = "This is a small modal subtitle"}},
    //     .fontFamily = ui::FontFamily::TEXT,
    //     .fontSize = sdl2w::TEXT_SIZE_16,
    //     .fontColor = ui::Colors::White,
    // });
    // modalLayout->setSubtitleElement(subtitle.release());

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(modalSmall));

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
      argc, argv, TestUiParams{800, 600, "ModalSmall Test"}, _init, _updateRender, [&]() {
        elements.clear();
      });
  LOG(INFO) << "End ModalSmall test" << LOG_ENDL;
  return 0;
}

#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalSmall.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"

namespace {

constexpr const char* kPortraitSprite = "portraits0_2";

ui::ModalSmall* makeModalSmall(sdl2w::Window& window,
                               int x,
                               int y,
                               int width,
                               int height,
                               float iconScale,
                               const char* titleText) {
  auto* modal = new ui::ModalSmall(&window);
  modal->setPos(x, y);
  modal->setScale(1.f);
  modal->setProps(ui::ModalSmallProps{
      .width = width,
      .height = height,
      .layoutFit = ui::LayoutFit::FullBleed,
      .backgroundColor = ui::Colors::ModalStandardBackground,
      .iconSprite = kPortraitSprite,
      .iconScale = iconScale,
      .enableCloseButton = true,
  });

  auto* title = new ui::TextLine(&window, modal);
  ui::TextFontProps titleFont;
  ui::setBaseFontConfig(titleFont, ui::BaseFontConfig::MODAL_TITLE);
  ui::TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = sdl2w::TEXT_SIZE_24;
  titleProps.fontColor = ui::Colors::Black;
  titleProps.textAlign = ui::TextAlign::LEFT_TOP;
  titleProps.textBlocks.pushBack({.text = titleText});
  title->setProps(titleProps);
  modal->setTitleElement(title);

  return modal;
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start ModalSmall test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& /*store*/) {
    LOG(INFO) << "ModalSmall test initialized (iconScale 1 vs 2)" << LOG_ENDL;

    constexpr int modalW = 360;
    constexpr int modalH = 280;
    constexpr int gap = 24;
    constexpr int leftX = 40;
    constexpr int rightX = leftX + modalW + gap;
    constexpr int topY = 60;

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(
        makeModalSmall(window, leftX, topY, modalW, modalH, 1.f, "iconScale = 1")));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(
        makeModalSmall(window, rightX, topY, modalW, modalH, 2.f, "iconScale = 2")));

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

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& /*store*/) {
    auto& events = window.getEvents();
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(events.mouseX, events.mouseY);
      }
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& /*store*/) {
    auto& draw = window.getDraw();
    draw.clearScreen();
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
              TestUiParams{800, 600, "ModalSmall iconScale Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End ModalSmall test" << LOG_ENDL;
  return 0;
}

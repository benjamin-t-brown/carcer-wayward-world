#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/VerticalList.h"
#include "ui/elements/TextLine.h"
#include <memory>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start VerticalList test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "VerticalList test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    auto verticalList = new ui::VerticalList(&window);

    auto& style = verticalList->getStyle();
    style.width = 400;
    style.x = (windowWidth - 400) / 2;
    style.y = (windowHeight - 300) / 2;

    ui::VerticalListProps props;
    props.lineHeight = 30;
    props.lineGap = 2;
    props.bgColor = ui::Colors::White;

    for (int i = 0; i < 8; ++i) {
      auto quad = new ui::Quad(&window, verticalList);
      auto& quadStyle = quad->getStyle();
      quadStyle.width = style.width;
      quadStyle.height = props.lineHeight;
      quadStyle.scale = 1.;
      quad->setProps(ui::QuadProps{
          .bgColor = ui::Colors::LightGrey,
      });

      auto textLine = new ui::TextLine(&window, quad);
      auto& textStyle = textLine->getStyle();
      ui::setBaseFontConfig(textStyle, ui::BaseFontConfig::MODAL_TEXT);
      textStyle.fontColor = ui::Colors::Black;
      textStyle.textAlign = ui::TextAlign::LEFT_TOP;

      ui::TextLineProps textProps;
      ui::TextBlock textBlock;
      textBlock.text = "List Item " + bmin::toString(i + 1);
      textProps.textBlocks.pushBack(textBlock);
      textLine->setProps(textProps);

      quad->addChild(textLine);
      verticalList->addListItem(quad);
    }

    verticalList->setProps(props);

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(verticalList));

    auto& events = window.getEvents();
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                         [&](int x, int y, int button) {
                           LOG(INFO) << "Mouse down at: " << x << ", " << y
                                     << " - button: " << button << LOG_ENDL;
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

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
      }
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
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

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "VerticalList Test"}, _init, _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End VerticalList test" << LOG_ENDL;
  return 0;
}

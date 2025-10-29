#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/components/VerticalList.h"
#include "ui/elements/TextLine.h"
#include "ui/colors.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start VerticalList test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "VerticalList test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();
    
    // Create VerticalList component
    auto verticalList = std::make_unique<ui::VerticalList>(&window);
    ui::BaseStyle style;
    style.width = 400;
    style.height = 300;
    style.x = (windowWidth - 400) / 2;
    style.y = (windowHeight - 300) / 2;
    verticalList->setStyle(style);

    // Configure VerticalList properties
    ui::VerticalListProps props;
    props.lineHeight = 30;
    props.lineSpacing = 2;
    props.borderSize = 1;
    props.lineBackgroundColor = ui::Colors::White;
    props.lineBorderColor = ui::Colors::Black;
    verticalList->setProps(props);

    // Add some text elements as children
    for (int i = 0; i < 8; ++i) {
      auto textLine = std::make_unique<ui::TextLine>(&window, verticalList.get());
      ui::BaseStyle textStyle;
      textStyle.fontFamily = ui::FontFamily::PARAGRAPH;
      textStyle.fontSize = sdl2w::TEXT_SIZE_16;
      textStyle.fontColor = ui::Colors::Black;
      textStyle.textAlign = ui::TextAlign::LEFT_CENTER;
      textLine->setStyle(textStyle);
      
      // Set text using props
      ui::TextLineProps textProps;
      ui::TextBlock textBlock;
      textBlock.text = "List Item " + std::to_string(i + 1);
      textProps.textBlocks.push_back(textBlock);
      textLine->setProps(textProps);
      
      verticalList->getChildren().push_back(std::move(textLine));
    }

    // Rebuild the list to position children correctly
    verticalList->build();

    elements.push_back(std::move(verticalList));

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
    events.setMouseEvent(
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

    // Check hover events for all elements
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
      argc, argv, TestUiParams{800, 600, "VerticalList Test"}, _init, _updateRender);
  LOG(INFO) << "End VerticalList test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

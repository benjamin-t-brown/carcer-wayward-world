#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "ui/layouts/ModalStandard.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start ModalStandard test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ModalStandard test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    // Create ModalStandard layout
    auto modalLayout = std::make_unique<ui::ModalStandard>(&window);
    ui::BaseStyle layoutStyle;
    layoutStyle.width = windowWidth;
    layoutStyle.height = windowHeight;
    layoutStyle.x = 0;
    layoutStyle.y = 0;
    modalLayout->setStyle(layoutStyle);
    // Set layout properties
    ui::ModalStandardProps props;
    props.backgroundColor = ui::Colors::ModalStandardBackground;
    props.decorationSprite = "";
    modalLayout->setProps(props);

    // Create title and subtitle elements
    auto title = std::make_unique<ui::TextLine>(&window);
    ui::BaseStyle titleStyle;
    titleStyle.fontFamily = ui::FontFamily::H1;
    titleStyle.fontSize = sdl2w::TEXT_SIZE_24;
    titleStyle.fontColor = ui::Colors::White;
    title->setStyle(titleStyle);
    ui::TextLineProps titleProps;
    ui::TextBlock titleBlock;
    titleBlock.text = "Modal Title";
    titleProps.textBlocks.push_back(titleBlock);
    title->setProps(titleProps);
    modalLayout->setTitleElement(std::move(title));

    auto subtitle = std::make_unique<ui::TextLine>(&window);
    ui::BaseStyle subtitleStyle;
    subtitleStyle.fontFamily = ui::FontFamily::PARAGRAPH;
    subtitleStyle.fontSize = sdl2w::TEXT_SIZE_16;
    subtitleStyle.fontColor = ui::Colors::White;
    subtitle->setStyle(subtitleStyle);
    ui::TextLineProps subtitleProps;
    ui::TextBlock subtitleBlock;
    subtitleBlock.text = "This is a subtitle";
    subtitleProps.textBlocks.push_back(subtitleBlock);
    subtitle->setProps(subtitleProps);
    modalLayout->setSubtitleElement(std::move(subtitle));

    elements.push_back(std::move(modalLayout));

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
      argc, argv, TestUiParams{800, 600, "ModalStandard Test"}, _init, _updateRender);
  LOG(INFO) << "End ModalStandard test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

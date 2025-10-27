#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "types/WorldActions.h"
#include "ui/UiElement.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/layouts/InGameLayout.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start InGameLayout test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "InGameLayout test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();
    // Create InGameLayout component
    auto inGameLayout = std::make_unique<ui::InGameLayout>(&window);
    ui::BaseStyle style;
    style.width = windowWidth;
    style.height = windowHeight;
    style.x = 0;
    style.y = 0;
    inGameLayout->setStyle(style);
    ui::InGameLayoutProps inGameLayoutProps;
    inGameLayoutProps.worldActionTypes = {
        types::WorldActionType::EXAMINE,
        types::WorldActionType::TALK,
        types::WorldActionType::ABILITY,
        types::WorldActionType::SNEAK,
        // types::WorldActionType::UNLOCK,
        types::WorldActionType::JUMP,
        types::WorldActionType::GET,
        types::WorldActionType::START_FIGHT,
        types::WorldActionType::INTERACT,
        types::WorldActionType::SHOOT,
        types::WorldActionType::JOURNAL,
        types::WorldActionType::INVENTORY,
        types::WorldActionType::STATUS,
        types::WorldActionType::MAP,
    };
    inGameLayout->setProps(inGameLayoutProps);

    // Create title element
    auto titleElement = std::make_unique<ui::TextLine>(&window);
    ui::BaseStyle titleStyle;
    titleStyle.width = 200;
    titleStyle.fontColor = ui::Colors::White;
    titleElement->setStyle(titleStyle);
    ui::TextLineProps titleProps;
    ui::TextBlock titleBlock;
    titleBlock.text = "Game Title";
    titleBlock.fontSize = sdl2w::TextSize::TEXT_SIZE_28;
    titleProps.textBlocks.push_back(titleBlock);
    titleElement->setProps(titleProps);

    // Create subtitle element
    auto subtitleElement = std::make_unique<ui::TextParagraph>(&window);
    ui::BaseStyle subtitleStyle;
    subtitleStyle.width = 200;
    subtitleStyle.height = 20;
    subtitleStyle.fontColor = ui::Colors::White;
    subtitleElement->setStyle(subtitleStyle);
    ui::TextParagraphProps subtitleProps;
    ui::TextBlock subtitleBlock;
    subtitleBlock.text = "Subtitle Text";
    subtitleBlock.fontSize = sdl2w::TextSize::TEXT_SIZE_24;
    subtitleProps.textBlocks.push_back(subtitleBlock);
    subtitleElement->setProps(subtitleProps);

    // Set title and subtitle elements
    inGameLayout->setTitleElement(std::move(titleElement));
    inGameLayout->setSubtitleElement(std::move(subtitleElement));

    elements.push_back(std::move(inGameLayout));

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
      element->render();
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "InGameLayout Test"}, _init, _updateRender);
  LOG(INFO) << "End InGameLayout test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"

#include "../setupTestUi.h"
#include "ui/UiElement.h"
#include "ui/elements/ButtonModal.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonModal test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonModal test initialized" << LOG_ENDL;

    // Create first button (not selected)
    auto button1 = std::make_unique<ui::ButtonModal>(&window);
    ui::BaseStyle button1Style;
    button1Style.x = 50;
    button1Style.y = 50;
    button1Style.width = 200;
    button1Style.height = 50;
    button1->setStyle(button1Style);
    ui::ButtonModalProps button1Props;
    button1Props.text = "Button 1 (Normal)";
    button1Props.isSelected = false;
    button1->setProps(button1Props);
    elements.push_back(std::move(button1));

    // Create second button (selected)
    auto button2 = std::make_unique<ui::ButtonModal>(&window);
    ui::BaseStyle button2Style;
    button2Style.x = 50;
    button2Style.y = 120;
    button2Style.width = 200;
    button2Style.height = 50;
    button2->setStyle(button2Style);
    ui::ButtonModalProps button2Props;
    button2Props.text = "Button 2 (Selected)";
    button2Props.isSelected = true;
    button2->setProps(button2Props);
    elements.push_back(std::move(button2));

    // Create third button (different size)
    auto button3 = std::make_unique<ui::ButtonModal>(&window);
    ui::BaseStyle button3Style;
    button3Style.x = 50;
    button3Style.y = 190;
    button3Style.width = 300;
    button3Style.height = 60;
    button3->setStyle(button3Style);
    ui::ButtonModalProps button3Props;
    button3Props.text = "Wide Button";
    button3Props.isSelected = false;
    button3->setProps(button3Props);
    elements.push_back(std::move(button3));

    // Create fourth button (scaled)
    auto button4 = std::make_unique<ui::ButtonModal>(&window);
    ui::BaseStyle button4Style;
    button4Style.x = 50;
    button4Style.y = 270;
    button4Style.width = 150;
    button4Style.height = 40;
    button4Style.scale = 2.f;
    button4->setStyle(button4Style);
    ui::ButtonModalProps button4Props;
    button4Props.text = "Scaled Button";
    button4Props.isSelected = true;
    button4->setProps(button4Props);
    elements.push_back(std::move(button4));

    // Create fifth button (empty text)
    auto button5 = std::make_unique<ui::ButtonModal>(&window);
    ui::BaseStyle button5Style;
    button5Style.x = 50;
    button5Style.y = 360;
    button5Style.width = 200;
    button5Style.height = 50;
    button5->setStyle(button5Style);
    ui::ButtonModalProps button5Props;
    button5Props.text = ""; // Empty text
    button5Props.isSelected = false;
    button5->setProps(button5Props);
    elements.push_back(std::move(button5));

    // Create sixth button (long text)
    auto button6 = std::make_unique<ui::ButtonModal>(&window);
    ui::BaseStyle button6Style;
    button6Style.x = 50;
    button6Style.y = 420;
    button6Style.width = 250;
    button6Style.height = 50;
    button6->setStyle(button6Style);
    ui::ButtonModalProps button6Props;
    button6Props.text = "This is selected.";
    button6Props.isSelected = true;
    button6->setProps(button6Props);
    elements.push_back(std::move(button6));
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    for (auto& elem : elements) {
      if (elem) {
        elem->render();
      }
    }
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{640, 600, "ButtonModal Test"}, _init, _updateRender);
  LOG(INFO) << "End ButtonModal test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

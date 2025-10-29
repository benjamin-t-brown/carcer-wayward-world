#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/colors.h"
#include "ui/elements/ButtonTextWrap.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

class TestButtonTextWrapObserver : public ui::UiEventObserver {
  std::string id;

public:
  TestButtonTextWrapObserver(std::string _id);
  ~TestButtonTextWrapObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonTextWrapObserver::TestButtonTextWrapObserver(std::string _id) : id(_id) {}

void TestButtonTextWrapObserver::onMouseDown(int x, int y, int button) {
  LOG(INFO) << "TestButtonTextWrapObserver mousedown at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonTextWrapObserver::onMouseUp(int x, int y, int button) {
  LOG(INFO) << "TestButtonTextWrapObserver mouseup at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonTextWrapObserver::onClick(int x, int y, int button) {
  LOG(INFO) << "TestButtonTextWrapObserver click at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonTextWrap test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonTextWrap test initialized" << LOG_ENDL;

    // Create first button with short text
    auto button1 = std::make_unique<ui::ButtonTextWrap>(&window);
    button1->setId("button1");
    ui::BaseStyle button1Style;
    button1Style.x = 50;
    button1Style.y = 50;
    button1Style.width = 200;
    // button1Style.fontColor = ui::Colors::Black;
    button1->setStyle(button1Style);
    ui::ButtonTextWrapProps button1Props;
    button1Props.text = "Short Text";
    button1Props.isSelected = false;
    button1->setProps(button1Props);
    button1->addEventObserver(std::make_unique<TestButtonTextWrapObserver>("button1"));
    elements.push_back(std::move(button1));

    // Create second button with long text that wraps
    auto button2 = std::make_unique<ui::ButtonTextWrap>(&window);
    button2->setId("button2");
    ui::BaseStyle button2Style;
    button2Style.x = 50;
    button2Style.y = 130;
    button2Style.width = 300;
    button2Style.fontColor = ui::Colors::Black;

    button2->setStyle(button2Style);
    ui::ButtonTextWrapProps button2Props;
    button2Props.text = "This is a much longer text that should wrap to multiple lines "
                        "when it exceeds the button width. Lol wow that's amazing bro.";
    button2Props.isSelected = true;
    button2Props.horizontalPadding = 10;
    button2Props.verticalPadding = 10;
    button2->setProps(button2Props);
    button2->addEventObserver(std::make_unique<TestButtonTextWrapObserver>("button2"));
    elements.push_back(std::move(button2));

    // Create third button with medium text
    auto button3 = std::make_unique<ui::ButtonTextWrap>(&window);
    button3->setId("button3");
    ui::BaseStyle button3Style;
    button3Style.x = 50;
    button3Style.y = 230;
    button3Style.width = 250;
    button3Style.fontColor = ui::Colors::Black;
    button3->setStyle(button3Style);
    ui::ButtonTextWrapProps button3Props;
    button3Props.text = "Medium length text that might wrap";
    button3Props.isSelected = false;
    button3->setProps(button3Props);
    button3->addEventObserver(std::make_unique<TestButtonTextWrapObserver>("button3"));
    elements.push_back(std::move(button3));

    // Create fourth button with very long text
    auto button4 = std::make_unique<ui::ButtonTextWrap>(&window);
    button4->setId("button4");
    ui::BaseStyle button4Style;
    button4Style.x = 50;
    button4Style.y = 320;
    button4Style.width = 400;
    button4->setStyle(button4Style);
    ui::ButtonTextWrapProps button4Props;
    button4Props.text = "This is an extremely long text that will definitely wrap to "
                        "multiple lines and should demonstrate the text wrapping "
                        "functionality of the ButtonTextWrap element very well";
    button4Props.isSelected = false;
    button4->setProps(button4Props);
    button4->addEventObserver(std::make_unique<TestButtonTextWrapObserver>("button4"));
    elements.push_back(std::move(button4));

    // Create fifth button with single line text
    auto button5 = std::make_unique<ui::ButtonTextWrap>(&window);
    button5->setId("button5");
    ui::BaseStyle button5Style;
    button5Style.x = 50;
    button5Style.y = 440;
    button5Style.width = 150;
    button5Style.scale = 2.f; // Test scaling
    button5->setStyle(button5Style);
    ui::ButtonTextWrapProps button5Props;
    button5Props.text = "Scaled Button";
    button5Props.isSelected = true;
    button5->setProps(button5Props);
    button5->addEventObserver(std::make_unique<TestButtonTextWrapObserver>("button5"));
    elements.push_back(std::move(button5));

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
    window.getDraw().setBackgroundColor({200, 200, 200});

    // Get mouse position for hover events
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    // Check hover events for all buttons
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
        elem->render(window.getDeltaTime());
      }
    }

    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{600, 600, "ButtonTextWrap Test"}, _init, _updateRender);
  LOG(INFO) << "End ButtonTextWrap test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/ButtonModal.h"
#include "ui/elements/ButtonScroll.h"
#include "ui/elements/ButtonClose.h"
#include <SDL2/SDL_pixels.h>
#include <memory>

class TestButtonObserver : public ui::UiEventObserver {
  std::string id;

public:
  TestButtonObserver(std::string _id);
  ~TestButtonObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonObserver::TestButtonObserver(std::string _id) : id(_id) {}

void TestButtonObserver::onMouseDown(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver mousedown at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonObserver::onMouseUp(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver mouseup at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

void TestButtonObserver::onClick(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver click at: " << x << ", " << y
            << " - button: " << button << LOG_ENDL;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start ButtonModal test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonModal test initialized" << LOG_ENDL;

    // Create first button (not selected)
    auto button1 = std::make_unique<ui::ButtonModal>(&window);
    button1->setId("button1");
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
    button1->addEventObserver(std::make_unique<TestButtonObserver>("button1"));
    elements.push_back(std::move(button1));

    // Create second button (selected)
    auto button2 = std::make_unique<ui::ButtonModal>(&window);
    button2->setId("button2");

    button2->addEventObserver(std::make_unique<TestButtonObserver>("button2"));

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
    button2->addEventObserver(std::make_unique<TestButtonObserver>("button2"));
    elements.push_back(std::move(button2));

    // Create third button (different size)
    auto button3 = std::make_unique<ui::ButtonModal>(&window);
    button3->setId("button3");
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
    button3->addEventObserver(std::make_unique<TestButtonObserver>("button3"));
    elements.push_back(std::move(button3));

    // Create fourth button (scaled)
    auto button4 = std::make_unique<ui::ButtonModal>(&window);
    button4->setId("button4");
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
    button4->addEventObserver(std::make_unique<TestButtonObserver>("button4"));
    elements.push_back(std::move(button4));

    // Create fifth button (empty text)
    auto button5 = std::make_unique<ui::ButtonModal>(&window);
    button5->setId("button5");
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
    button5->addEventObserver(std::make_unique<TestButtonObserver>("button5"));
    elements.push_back(std::move(button5));

    // Create sixth button (long text)
    auto button6 = std::make_unique<ui::ButtonModal>(&window);
    button6->setId("button6");
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
    button6->addEventObserver(std::make_unique<TestButtonObserver>("button6"));
    elements.push_back(std::move(button6));

    // Create scroll up button
    auto scrollUp = std::make_unique<ui::ButtonScroll>(&window);
    scrollUp->setId("scrollUp");
    ui::BaseStyle scrollUpStyle;
    scrollUpStyle.x = 350;
    scrollUpStyle.y = 50;
    // ButtonScroll defaults to 32x32, but we can override if needed
    scrollUpStyle.width = 32;
    scrollUpStyle.height = 32;
    scrollUp->setStyle(scrollUpStyle);
    ui::ButtonScrollProps scrollUpProps;
    scrollUpProps.direction = ui::ScrollDirection::UP;
    scrollUpProps.isSelected = false;
    scrollUp->setProps(scrollUpProps);
    scrollUp->addEventObserver(std::make_unique<TestButtonObserver>("scrollUp"));
    elements.push_back(std::move(scrollUp));

    // Create scroll down button
    auto scrollDown = std::make_unique<ui::ButtonScroll>(&window);
    scrollDown->setId("scrollDown");
    ui::BaseStyle scrollDownStyle;
    scrollDownStyle.x = 350;
    scrollDownStyle.y = 100;
    scrollDownStyle.width = 32;
    scrollDownStyle.height = 32;
    scrollDown->setStyle(scrollDownStyle);
    ui::ButtonScrollProps scrollDownProps;
    scrollDownProps.direction = ui::ScrollDirection::DOWN;
    scrollDownProps.isSelected = true; // Make this one selected to show the selection state
    scrollDown->setProps(scrollDownProps);
    scrollDown->addEventObserver(std::make_unique<TestButtonObserver>("scrollDown"));
    elements.push_back(std::move(scrollDown));

    // Create modal close button
    auto modalClose = std::make_unique<ui::ButtonClose>(&window);
    modalClose->setId("modalClose");
    ui::BaseStyle modalCloseStyle;
    modalCloseStyle.x = 400;
    modalCloseStyle.y = 50;
    modalCloseStyle.width = 32;
    modalCloseStyle.height = 32;
    modalClose->setStyle(modalCloseStyle);
    ui::ButtonCloseProps modalCloseProps;
    modalCloseProps.closeType = ui::CloseType::MODAL;
    modalClose->setProps(modalCloseProps);
    modalClose->addEventObserver(std::make_unique<TestButtonObserver>("modalClose"));
    elements.push_back(std::move(modalClose));

    // Create popup close button
    auto popupClose = std::make_unique<ui::ButtonClose>(&window);
    popupClose->setId("popupClose");
    ui::BaseStyle popupCloseStyle;
    popupCloseStyle.x = 400;
    popupCloseStyle.y = 100;
    popupCloseStyle.width = 32;
    popupCloseStyle.height = 32;
    popupClose->setStyle(popupCloseStyle);
    ui::ButtonCloseProps popupCloseProps;
    popupCloseProps.closeType = ui::CloseType::POPUP;
    popupClose->setProps(popupCloseProps);
    popupClose->addEventObserver(std::make_unique<TestButtonObserver>("popupClose"));
    elements.push_back(std::move(popupClose));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
          LOG(INFO) << "Mouse down at: " << x << ", " << y << " - button: " << button << LOG_ENDL;
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
    // Get mouse position for hover events
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    // Check hover events for all buttons
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
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

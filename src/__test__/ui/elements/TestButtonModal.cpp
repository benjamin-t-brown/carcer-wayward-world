#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonScroll.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <memory>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

class TestButtonObserver : public ui::UiEventObserver {
  bmin::String id;

public:
  TestButtonObserver(bmin::String _id);
  ~TestButtonObserver() override = default;
  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onClick(int x, int y, int button) override;
};

TestButtonObserver::TestButtonObserver(bmin::String _id) : id(_id) {}

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
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "ButtonModal test initialized" << LOG_ENDL;

    // Create first button (not selected)
    auto button1 = bmin::makeUnique<ui::ButtonModal>(&window);
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
    button1->addEventObserver(new TestButtonObserver("button1"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button1.release()));

    // Create second button (selected)
    auto button2 = bmin::makeUnique<ui::ButtonModal>(&window);
    button2->setId("button2");

    button2->addEventObserver(new TestButtonObserver("button2"));

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
    button2->addEventObserver(new TestButtonObserver("button2"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button2.release()));

    // Create third button (different size)
    auto button3 = bmin::makeUnique<ui::ButtonModal>(&window);
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
    button3->addEventObserver(new TestButtonObserver("button3"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button3.release()));

    // Create fourth button (scaled)
    auto button4 = bmin::makeUnique<ui::ButtonModal>(&window);
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
    button4->addEventObserver(new TestButtonObserver("button4"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button4.release()));

    // Create fifth button (empty text)
    auto button5 = bmin::makeUnique<ui::ButtonModal>(&window);
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
    button5->addEventObserver(new TestButtonObserver("button5"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button5.release()));

    // Create sixth button (long text)
    auto button6 = bmin::makeUnique<ui::ButtonModal>(&window);
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
    button6->addEventObserver(new TestButtonObserver("button6"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(button6.release()));

    // Create scroll up button
    auto scrollUp = bmin::makeUnique<ui::ButtonScroll>(&window);
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
    scrollUp->addEventObserver(new TestButtonObserver("scrollUp"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(scrollUp.release()));

    // Create scroll down button
    auto scrollDown = bmin::makeUnique<ui::ButtonScroll>(&window);
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
    scrollDown->addEventObserver(new TestButtonObserver("scrollDown"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(scrollDown.release()));

    // Create modal close button
    auto modalClose = bmin::makeUnique<ui::ButtonClose>(&window);
    modalClose->setId("modalClose");
    auto& style = modalClose->getStyle();
    style.x = 400;
    style.y = 50;
    ui::ButtonCloseProps modalCloseProps;
    modalCloseProps.closeType = ui::CloseType::MODAL;
    modalClose->setProps(modalCloseProps);
    modalClose->addEventObserver(new TestButtonObserver("modalClose"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(modalClose.release()));

    {
      auto modalClose = bmin::makeUnique<ui::ButtonClose>(&window);
      modalClose->setId("modalCloseScaled");
      auto& style = modalClose->getStyle();
      style.x = 475;
      style.y = 50;
      style.scale = 2.f;
      ui::ButtonCloseProps modalCloseProps;
      modalCloseProps.closeType = ui::CloseType::MODAL;
      modalClose->setProps(modalCloseProps);
      modalClose->addEventObserver(new TestButtonObserver(modalClose->getId()));
      elements.pushBack(bmin::UniquePtr<ui::UiElement>(modalClose.release()));
    }

    // Create popup close button
    auto popupClose = bmin::makeUnique<ui::ButtonClose>(&window);
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
    popupClose->addEventObserver(new TestButtonObserver("popupClose"));
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(popupClose.release()));

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
        elem->render(window.getDeltaTime());
      }
    }

    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{640, 600, "ButtonModal Test"}, _init, _updateRender, [&]() { elements.clear(); });
  LOG(INFO) << "End ButtonModal test" << LOG_ENDL;
  return 0;
}

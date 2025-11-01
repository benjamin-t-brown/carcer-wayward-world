#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/UiElement.h"
#include "ui/elements/ButtonModal.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
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
            << " - button: " << button << " - id: " << id << LOG_ENDL;
}

void TestButtonObserver::onMouseUp(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver mouseup at: " << x << ", " << y
            << " - button: " << button << " - id: " << id << LOG_ENDL;
}

void TestButtonObserver::onClick(int x, int y, int button) {
  LOG(INFO) << "TestButtonObserver click at: " << x << ", " << y
            << " - button: " << button << " - id: " << id << LOG_ENDL;
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start SectionScrollable test" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "SectionScrollable test initialized" << LOG_ENDL;

    // Create scrollable section
    auto scrollableSection = std::make_unique<ui::SectionScrollable>(&window);
    scrollableSection->setId("scrollableSection");
    ui::BaseStyle sectionStyle;
    sectionStyle.x = 100;
    sectionStyle.y = 100;
    sectionStyle.width = 400;
    sectionStyle.height = 300;
    scrollableSection->setStyle(sectionStyle);

    ui::SectionScrollableProps sectionProps;
    sectionProps.scrollBarWidth = 20;
    sectionProps.borderColor = ui::Colors::Black;
    sectionProps.borderSize = 2;
    sectionProps.scrollStep = 30;
    scrollableSection->setProps(sectionProps);

    auto quad1 = std::make_unique<ui::Quad>(&window);
    quad1->setId("quad1");
    ui::BaseStyle quad1Style;
    quad1Style.x = 0;
    quad1Style.y = 0;
    quad1Style.width = 300;
    quad1Style.height = 300;
    quad1->setStyle(quad1Style);
    ui::QuadProps quad1Props;
    quad1Props.bgColor = ui::Colors::Red;
    quad1Props.borderColor = ui::Colors::Transparent;
    quad1Props.borderSize = 0;
    quad1->setProps(quad1Props);
    scrollableSection->addChild(std::move(quad1));

    auto quad2 = std::make_unique<ui::Quad>(&window);
    quad2->setId("quad2");
    ui::BaseStyle quad2Style;
    quad2Style.x = 0;
    quad2Style.y = 300;
    quad2Style.width = 300;
    quad2Style.height = 300;
    quad2->setStyle(quad2Style);
    ui::QuadProps quad2Props;
    quad2Props.bgColor = ui::Colors::Blue;
    quad2Props.borderColor = ui::Colors::Transparent;
    quad2Props.borderSize = 0;
    quad2->setProps(quad2Props);
    scrollableSection->addChild(std::move(quad2));

    // Add test content to the scrollable section
    // for (int i = 0; i < 10; i++) {
    //   auto testButton = std::make_unique<ui::ButtonModal>(&window);
    //   testButton->setId("testButton" + std::to_string(i));
    //   ui::BaseStyle buttonStyle;
    //   buttonStyle.x = 20;
    //   buttonStyle.y = i * 60; // Stack buttons vertically
    //   buttonStyle.width = 300;
    //   buttonStyle.height = 50;
    //   testButton->setStyle(buttonStyle);

    //   ui::ButtonModalProps buttonProps;
    //   buttonProps.text = "Test Button " + std::to_string(i + 1);
    //   buttonProps.isSelected = (i % 3 == 0); // Every 3rd button is selected
    //   testButton->setProps(buttonProps);
    //   testButton->addEventObserver(
    //       std::make_unique<TestButtonObserver>("testButton" + std::to_string(i)));

    //   scrollableSection->addChild(std::move(testButton));
    // }

    // // Add some text elements as well
    // for (int i = 0; i < 5; i++) {
    //   auto textLine = std::make_unique<ui::TextLine>(&window);
    //   textLine->setId("textLine" + std::to_string(i));
    //   ui::BaseStyle textStyle;
    //   textStyle.x = 20;
    //   textStyle.y = 600 + (i * 40); // Position below the buttons
    //   textStyle.width = 300;
    //   textStyle.height = 30;
    //   textStyle.fontSize = sdl2w::TEXT_SIZE_16;
    //   textStyle.fontColor = ui::Colors::Black;
    //   textLine->setStyle(textStyle);

    //   ui::TextLineProps textProps;
    //   textProps.textBlocks = {ui::TextBlock{"This is test text line " +
    //   std::to_string(i + 1)}}; textLine->setProps(textProps);

    //   scrollableSection->addChild(std::move(textLine));
    // }

    elements.push_back(std::move(scrollableSection));

    auto& events = window.getEvents();
    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
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

    events.setMouseEvent(
        //
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
        [&](int x, int y, int delta) {
          for (auto& elem : elements) {
            LOG(INFO) << "Mouse wheel event at: " << x << ", " << y
                      << " - delta: " << delta << LOG_ENDL;
            elem->checkMouseWheelEvent(x, y, delta);
          }
        });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    window.getDraw().setBackgroundColor({70, 70, 70});

    // Get mouse position for hover events
    auto& events = window.getEvents();
    auto mouseX = events.mouseX;
    auto mouseY = events.mouseY;

    // Check hover events for all elements
    for (auto& elem : elements) {
      if (elem) {
        elem->checkHoverEvent(mouseX, mouseY);
        elem->render(window.getDeltaTime());
      }
    }

    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{600, 500, "SectionScrollable Test"}, _init, _updateRender);
  LOG(INFO) << "End SectionScrollable test" << LOG_ENDL;
  sdl2w::Window::unInit();
  return 0;
}

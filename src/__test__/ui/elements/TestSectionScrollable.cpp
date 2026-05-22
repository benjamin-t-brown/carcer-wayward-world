#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include <memory>
#include <vector>

int main(int argc, char** argv) {
  LOG(INFO) << "Start SectionScrollable test" << LOG_ENDL;
  srand(time(NULL));

  std::vector<std::unique_ptr<ui::UiElement>> elements;

  auto addTextToQuad =
      [&](sdl2w::Window& window, ui::Quad* quad, const std::string& text) {
        auto textLine = new ui::TextLine(&window, quad);
        auto& textStyle = textLine->getStyle();
        textStyle.x = quad->getStyle().width / 2.f;
        textStyle.y = quad->getStyle().height / 2.f;
        textStyle.textAlign = ui::TextAlign::CENTER;
        ui::setBaseFontConfig(textStyle, ui::BaseFontConfig::MODAL_CHOICE_TEXT);
        textStyle.scale = 1.;
        ui::TextLineProps textLineProps;
        textLineProps.textBlocks = {ui::TextBlock{text}};
        textLine->setProps(textLineProps);
        quad->addChild(textLine);
      };

  auto addScrollableRows =
      [&](sdl2w::Window& window,
          ui::SectionScrollable* section,
          const std::vector<std::pair<SDL_Color, std::string>>& rows) {
        int y = 0;
        for (const auto& [color, label] : rows) {
          auto row = new ui::Quad(&window);
          row->setId("row_" + label);
          auto& rowStyle = row->getStyle();
          rowStyle.x = 0;
          rowStyle.y = y;
          rowStyle.width = section->getStyle().width - section->getProps().scrollBarWidth;
          rowStyle.height = 80;
          ui::QuadProps rowProps;
          rowProps.bgColor = color;
          rowProps.borderColor = ui::Colors::Transparent;
          rowProps.borderSize = 0;
          addTextToQuad(window, row, label);
          row->setProps(rowProps);
          section->addChild(row);
          y += rowStyle.height;
        }
      };

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "SectionScrollable test initialized" << LOG_ENDL;

    // Test 1: two large panels (minimal overflow)
    auto scrollableSection = new ui::SectionScrollable(&window);
    scrollableSection->setId("scrollableSection");

    auto section1Scale = 1.5f;

    {
      auto quad1 = new ui::Quad(&window);
      quad1->setId("quad1");
      auto& quad1Style = quad1->getStyle();
      quad1Style.x = 0;
      quad1Style.y = 0;
      quad1Style.width = 300;
      quad1Style.height = 300;
      quad1Style.scale = section1Scale;
      ui::QuadProps quad1Props;
      quad1Props.bgColor = ui::Colors::Red;
      quad1Props.borderColor = ui::Colors::Transparent;
      quad1Props.borderSize = 0;
      addTextToQuad(window, quad1, "Hello World 1");
      quad1->setProps(quad1Props);
      scrollableSection->addChild(quad1);

      auto quad2 = new ui::Quad(&window);
      quad2->setId("quad2");
      auto& quad2Style = quad2->getStyle();
      quad2Style.x = 0;
      quad2Style.y = 300 * section1Scale;
      quad2Style.width = 300;
      quad2Style.height = 300;
      quad2Style.scale = section1Scale;
      ui::QuadProps quad2Props;
      quad2Props.bgColor = ui::Colors::Blue;
      quad2Props.borderColor = ui::Colors::Transparent;
      quad2Props.borderSize = 0;
      addTextToQuad(window, quad2, "Hello World 2");
      quad2->setProps(quad2Props);
      scrollableSection->addChild(quad2);
    }

    auto& sectionStyle = scrollableSection->getStyle();
    sectionStyle.x = 40;
    sectionStyle.y = 50;
    sectionStyle.width = 400;
    sectionStyle.height = 300;
    sectionStyle.scale = section1Scale;
    ui::SectionScrollableProps sectionProps;
    sectionProps.scrollBarWidth = 40;
    sectionProps.borderColor = ui::Colors::Black;
    sectionProps.borderSize = 0;
    sectionProps.scrollStep = 30;
    scrollableSection->setProps(sectionProps);

    elements.push_back(std::unique_ptr<ui::UiElement>(scrollableSection));

    // Test 2: bordered section with many rows (production-like scroll)
    // auto listSection = new ui::SectionScrollable(&window);
    // listSection->setId("listScrollableSection");
    // auto& listStyle = listSection->getStyle();
    // listStyle.x = 470;
    // listStyle.y = 50;
    // listStyle.width = 380;
    // listStyle.height = 300;

    // ui::SectionScrollableProps listProps;
    // listProps.scrollBarWidth = 40;
    // listProps.borderColor = ui::Colors::Black;
    // listProps.borderSize = 2;
    // listProps.scrollStep = 40;
    // listSection->setProps(listProps);

    // addScrollableRows(window,
    //                   listSection,
    //                   {
    //                       {ui::Colors::LightGrey, "Row 1"},
    //                       {ui::Colors::White, "Row 2"},
    //                       {ui::Colors::LightGrey, "Row 3"},
    //                       {ui::Colors::White, "Row 4"},
    //                       {ui::Colors::LightGrey, "Row 5"},
    //                       {ui::Colors::White, "Row 6"},
    //                   });
    // elements.push_back(std::unique_ptr<ui::UiElement>(listSection));

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

  setupTestUi(argc,
              argv,
              TestUiParams{900, 900, "SectionScrollable Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End SectionScrollable test" << LOG_ENDL;
  return 0;
}

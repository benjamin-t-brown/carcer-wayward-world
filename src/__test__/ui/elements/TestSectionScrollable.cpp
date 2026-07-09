#include "../../setupTestUi.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/TextStyle.h"
#include "ui/UiElement.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include <memory>
#include <vector>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start SectionScrollable test" << LOG_ENDL;
  srand(time(NULL));

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto addTextToQuad =
      [&](sdl2w::Window& window, ui::Quad* quad, const bmin::String& text) {
        const auto [quadW, quadH] = quad->getDims();
        auto textLine = new ui::TextLine(&window, quad);
        ui::TextFontProps font;
        ui::setBaseFontConfig(font, ui::BaseFontConfig::MODAL_CHOICE_TEXT);
        textLine->setPos(quadW / 2, quadH / 2);
        textLine->setScale(1.f);
        textLine->setProps(ui::TextLineProps{
            .textBlocks = {ui::TextBlock{text}},
            .fontFamily = font.fontFamily,
            .fontSize = font.fontSize,
            .fontColor = font.fontColor,
            .textAlign = ui::TextAlign::CENTER,
        });
        quad->addChild(textLine);
      };

  // auto addScrollableRows =
  //     [&](sdl2w::Window& window,
  //         ui::SectionScrollable* section,
  //         const bmin::DynArray<std::pair<SDL_Color, bmin::String>>& rows) {
  //       const int sectionWidth = section->getProps().width;
  //       const int scrollBarWidth = section->getProps().scrollBarWidth;
  //       const int rowWidth = sectionWidth - scrollBarWidth;
  //       const int rowHeight = 80;
  //       int y = 0;
  //       for (const auto& [color, label] : rows) {
  //         auto row = new ui::Quad(&window);
  //         row->setId("row_" + label);
  //         row->setPos(0, y);
  //         row->setProps(ui::QuadProps{
  //             .width = rowWidth,
  //             .height = rowHeight,
  //             .bgColor = color,
  //             .borderColor = ui::Colors::Transparent,
  //             .borderSize = 0,
  //         });
  //         addTextToQuad(window, row, label);
  //         section->addChild(row);
  //         y += rowHeight;
  //       }
  //     };

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "SectionScrollable test initialized" << LOG_ENDL;

    // Test 1: two large panels (minimal overflow)
    auto scrollableSection = new ui::SectionScrollable(&window);
    scrollableSection->setId("scrollableSection");

    auto section1Scale = 1.5f;

    {
      auto quad1 = new ui::Quad(&window);
      quad1->setId("quad1");
      quad1->setPos(0, 0);
      quad1->setScale(section1Scale);
      quad1->setProps(ui::QuadProps{
          .width = 300,
          .height = 300,
          .bgColor = ui::Colors::Red,
          .borderColor = ui::Colors::Transparent,
          .borderSize = 0,
      });
      addTextToQuad(window, quad1, "Hello World 1");
      scrollableSection->addChild(quad1);

      auto quad2 = new ui::Quad(&window);
      quad2->setId("quad2");
      quad2->setPos(0, static_cast<int>(300 * section1Scale));
      quad2->setScale(section1Scale);
      quad2->setProps(ui::QuadProps{
          .width = 300,
          .height = 300,
          .bgColor = ui::Colors::Blue,
          .borderColor = ui::Colors::Transparent,
          .borderSize = 0,
      });
      addTextToQuad(window, quad2, "Hello World 2");
      scrollableSection->addChild(quad2);
    }

    scrollableSection->setPos(40, 50);
    scrollableSection->setScale(section1Scale);
    scrollableSection->setProps(ui::SectionScrollableProps{
        .width = 400,
        .height = 300,
        .scrollBarWidth = 40,
        .borderColor = ui::Colors::Black,
        .borderSize = 0,
        .scrollStep = 30,
    });

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(scrollableSection));

    // Test 2: bordered section with many rows (production-like scroll)
    // auto listSection = new ui::SectionScrollable(&window);
    // listSection->setId("listScrollableSection");
    // listSection->setPos(470, 50);
    // listSection->setProps(ui::SectionScrollableProps{
    //     .width = 380,
    //     .height = 300,
    //     .scrollBarWidth = 40,
    //     .borderColor = ui::Colors::Black,
    //     .borderSize = 2,
    //     .scrollStep = 40,
    // });
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
    // elements.pushBack(bmin::UniquePtr<ui::UiElement>(listSection));

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

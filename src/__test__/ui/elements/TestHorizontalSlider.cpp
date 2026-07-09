#include "../../setupTestUi.h"
#include "lib/sdl2w/Events.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/elements/HorizontalSlider.h"
#include "ui/elements/TextLine.h"
#include <memory>
#include <vector>
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start HorizontalSlider test" << LOG_ENDL;

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;
  ui::HorizontalSlider* rangeSlider = nullptr;
  ui::HorizontalSlider* singleValueSlider = nullptr;
  ui::TextLine* valueReadout = nullptr;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "HorizontalSlider test initialized" << LOG_ENDL;

    auto sliderA = new ui::HorizontalSlider(&window);
    sliderA->setId("rangeSlider");
    auto& styleA = sliderA->getStyle();
    styleA.x = 60;
    styleA.y = 80;
    styleA.width = 420;
    styleA.scale = 1.f;
    sliderA->setProps({
        .minValue = 1,
        .maxValue = 10,
        .value = 4,
        .sliderBarHeight = 32,
        .indicatorWidth = 24,
        .labelColor = ui::Colors::White,
    });
    rangeSlider = sliderA;
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(sliderA));

    auto sliderB = new ui::HorizontalSlider(&window);
    sliderB->setId("singleValueSlider");
    auto& styleB = sliderB->getStyle();
    styleB.x = 60;
    styleB.y = 180;
    styleB.width = 420;
    styleB.scale = 1.f;
    sliderB->setProps({
        .minValue = 1,
        .maxValue = 2,
        .value = 1,
        .sliderBarHeight = 32,
        .indicatorWidth = 24,
    });
    singleValueSlider = sliderB;
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(sliderB));

    auto readout = new ui::TextLine(&window);
    readout->setId("valueReadout");
    auto& readoutStyle = readout->getStyle();
    ui::setBaseFontConfig(readoutStyle, ui::BaseFontConfig::MODAL_TEXT);
    readoutStyle.x = 60;
    readoutStyle.y = 260;
    readoutStyle.scale = 1.f;
    readoutStyle.textAlign = ui::TextAlign::LEFT_TOP;
    readout->setProps({
        .textBlocks =
            {
                {.text = "Slider value: 4"},
            },
    });
    valueReadout = readout;
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(readout));

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) {
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

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& events = window.getEvents();
    for (auto& elem : elements) {
      elem->checkHoverEvent(events.mouseX, events.mouseY);
      elem->render(window.getDeltaTime());
    }

    if (rangeSlider && valueReadout) {
      valueReadout->setProps({
          .textBlocks =
              {
                  {.text =
                       "Slider value: " + bmin::toString(rangeSlider->getProps().value)},
              },
      });
    }

    if (singleValueSlider) {
      const auto& sliderProps = singleValueSlider->getProps();
      if (sliderProps.value < sliderProps.minValue ||
          sliderProps.value > sliderProps.maxValue) {
        LOG(ERROR) << "singleValueSlider moved out of bounds" << LOG_ENDL;
      }
    }

    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{640, 480, "HorizontalSlider Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });

  LOG(INFO) << "End HorizontalSlider test" << LOG_ENDL;
  return 0;
}

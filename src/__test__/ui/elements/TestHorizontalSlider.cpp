#include "../../setupTestUi.h"
#include "sdl2w/Events.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/TextStyle.h"
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
    sliderA->setPos(60, 80);
    sliderA->setScale(1.f);
    sliderA->setProps({
        .minValue = 1,
        .maxValue = 10,
        .value = 4,
        .width = 420,
        .sliderBarHeight = 32,
        .indicatorWidth = 24,
        .labelColor = ui::Colors::White,
    });
    rangeSlider = sliderA;
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(sliderA));

    auto sliderB = new ui::HorizontalSlider(&window);
    sliderB->setId("singleValueSlider");
    sliderB->setPos(60, 180);
    sliderB->setScale(1.f);
    sliderB->setProps({
        .minValue = 1,
        .maxValue = 2,
        .value = 1,
        .width = 420,
        .sliderBarHeight = 32,
        .indicatorWidth = 24,
    });
    singleValueSlider = sliderB;
    elements.pushBack(bmin::UniquePtr<ui::UiElement>(sliderB));

    auto readout = new ui::TextLine(&window);
    readout->setId("valueReadout");
    ui::TextFontProps font;
    ui::setBaseFontConfig(font, ui::BaseFontConfig::MODAL_TEXT);
    readout->setPos(60, 260);
    readout->setScale(1.f);
    readout->setProps({
        .textBlocks =
            {
                {.text = "Slider value: 4"},
            },
        .fontFamily = font.fontFamily,
        .fontSize = font.fontSize,
        .fontColor = font.fontColor,
        .textAlign = ui::TextAlign::LEFT_TOP,
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
      auto props = valueReadout->getProps();
      props.textBlocks = {
          {.text = "Slider value: " + bmin::toString(rangeSlider->getProps().value)},
      };
      valueReadout->setProps(props);
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

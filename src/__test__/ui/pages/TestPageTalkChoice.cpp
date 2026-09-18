#include "../../setupTestUi.hpp"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "ui/SdlPixels.hpp" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/pages/PageTalkChoice.h"
#include <memory>
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Start PageTalkChoice test" << LOG_ENDL;
  srand(time(NULL));

  // Setup static classes
  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();
  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PageTalkChoice test initialized" << LOG_ENDL;

    auto [windowWidth, windowHeight] = window.getDims();

    auto pageTalkChoice = new ui::PageTalkChoice(&window, nullptr);
    pageTalkChoice->setId("pageTalkChoice");
    auto scale = 1.f;
    pageTalkChoice->setPos(0, 0);
    pageTalkChoice->setScale(scale);

    ui::PageTalkChoiceProps pageProps;
    pageProps.width = static_cast<int>(windowWidth / scale);
    pageProps.height = static_cast<int>(windowHeight / scale);
    pageProps.title = "Dockmaster Claire";
    pageProps.portraitSpriteName = "";
    // Unified log: history above the pin, current dialogue + in-flow choices below.
    // Enough wrapping rows that the last choice should clip at 640x480 (Show More).
    pageProps.choices = {
        {.nextId = "choice1",
         .text = "Ask what the harbor fees are this season, and whether they have gone "
                 "up again since the last storm.",
         .prefixText = ""},
        {.nextId = "choice2",
         .text = "Mention the missing crate from last week's shipment and watch her "
                 "reaction closely.",
         .prefixText = ""},
        {.nextId = "choice3",
         .text = "Offer to help unload the barge if she will talk about the night watch.",
         .prefixText = "[Special]"},
        {.nextId = "choice4",
         .text = "Ask whether the eastern quay is still closed to outsiders after dark.",
         .prefixText = "",
         .previouslyChosen = true},
        {.nextId = "choice5",
         .text = "Inquire about the captain who left without paying his docking bill.",
         .prefixText = ""},
        {.nextId = "choice6",
         .text = "Press her on the rumor that smugglers have been using the south pier "
                 "after midnight.",
         .prefixText = ""},
        {.nextId = "choice7",
         .text = "Ask if she has seen anyone matching the description of the missing "
                 "clerk from the counting house.",
         .prefixText = ""},
        {.nextId = "choice8",
         .text = "Change the subject and ask what she thinks of the new lighthouse tax.",
         .prefixText = ""},
        {.nextId = "choice9",
         .text = "Request a berth for three days and ask whether the inner basin still "
                 "has room for a small cutter.",
         .prefixText = ""},
        {.nextId = "choice10",
         .text = "Thank her for her time and ask if there is anything else a traveler "
                 "should know before nightfall.",
         .prefixText = ""},
        {.nextId = "choice11",
         .text = "Ask for the name of the watch sergeant who patrols the warehouses.",
         .prefixText = ""},
        {.nextId = "choice12",
         .text = "Wait in silence a moment longer, then ask what she is not telling you.",
         .prefixText = ""},
    };
    // clang-format off
    pageProps.textBlocks = {
      {.text ="According to all known laws of aviation, there is no way a bee should be able to fly.\n"},
      {.text ="Its wings are too small to get its fat little body off the ground.\n"},
      {.text ="The bee, of course, flies anyway because bees don't care what humans think is impossible.\n"},
      {.text ="Yellow, black. Yellow, black. Yellow, black. Yellow, black.\n"},
      {.text ="Ooh, black and yellow!\n"},
      {.text ="She says, \"Let's shake it up a little.\" And then smiles.\n"},
      {.text ="Barry! Breakfast is ready!\n"},
    };
    // clang-format on
    pageProps.pinFromBlockIndex = 3;
    pageTalkChoice->setProps(pageProps);

    elements.pushBack(bmin::UniquePtr<ui::UiElement>(pageTalkChoice));

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
            elem->checkMouseWheelEvent(x, y, delta);
          }
        });
  };

  auto _update = [&](sdl2w::Window& window, sdl2w::Store& store) {
    stateManager.update(window.getDeltaTime());
    for (auto& elem : elements) {
      elem->checkHoverEvent(window.getEvents().mouseX, window.getEvents().mouseY);
    }
  };

  auto _render = [&](sdl2w::Window& window, sdl2w::Store& store) {
    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();

    // Render all elements
    for (auto& elem : elements) {
      elem->render(window.getDeltaTime());
    }
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    _update(window, store);
    _render(window, store);
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{640, 480, "PageTalkChoice Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End PageTalkChoice test" << LOG_ENDL;
  return 0;
}

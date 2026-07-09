#include "../../setupTestUi.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"
#include "lib/sdl2w/Window.h"
#include "model/instances/CharacterPlayer.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/UiElement.h"
#include "ui/pages/PageInventory.h"
#include <memory>

int main(int argc, char** argv) {
  LOG(INFO) << "Start PageInventory test" << LOG_ENDL;
  srand(time(NULL));

  // Setup static classes
  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();
  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);

  DynArray<UniquePtr<ui::UiElement>> elements;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "PageInventory test initialized" << LOG_ENDL;

    DynArray<String> partyTemplateNames = {
        "testPartyMember1",
        "testPartyMember2",
        "testPartyMember3",
        "testPartyMember4",
    };

    auto& player = stateManager.getState().player;
    for (const auto& templateName : partyTemplateNames) {
      player.party.pushBack(
          model::CharacterPlayer(database.getCharacterTemplate(templateName)));
    }
    auto& characterPlayer = player.party.front();

    DynArray<String>
        //
        itemNames = {"PotionHealing",
                     "DaggerBronze",
                     "ShortSwordBronze",
                     "SwordBronze",
                     "LongbowOak",
                     "ArrowsStone",
                     "ShirtSimple0",
                     "ShirtSimple1",
                     "PantsSimple0",
                     "GlovesLeather",
                     "HatLeather",
                     "BootsLeather",
                     "NecklaceSilver"};

    for (const auto& itemName : itemNames) {
      model::characterPlayerAddItemToInventory(
          characterPlayer, database.getItemTemplate(itemName), 1);
    }

    auto [windowWidth, windowHeight] = window.getDims();

    // Create PageInventory component
    auto pageInventory = new ui::PageInventory(&window, nullptr);
    pageInventory->setId("pageInventory");
    auto& style = pageInventory->getStyle();

    auto scale = 1.f;
    style.width = windowWidth / scale;
    style.height = windowHeight / scale;
    style.x = 0;
    style.y = 0;
    style.scale = scale;

    player.gold = 1234;

    ui::PageInventoryProps pageProps;
    pageProps.characterPlayerId = characterPlayer.instanceId;
    pageProps.characterPlayerLabel = characterPlayer.params.label;
    pageProps.partyMemberInventoryIndex = player.currentPartyMemberInventoryIndex;
    for (const auto& member : player.party) {
      pageProps.partyMembers.pushBack(
          {.spriteName = model::characterPlayerGetSprite(member)});
    }
    pageProps.characterPlayerSprite = model::characterPlayerGetSprite(characterPlayer);
    pageProps.weightCarrying =
        model::characterGetWeightCarrying(characterPlayer, &database);
    pageProps.weightCapacity = model::characterGetWeightCapacity(characterPlayer);
    pageProps.gold = player.gold;
    pageProps.inventory = characterPlayer.inventory;
    pageInventory->setProps(pageProps);

    elements.pushBack(UniquePtr<ui::UiElement>(pageInventory));

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
              TestUiParams{400, 900, "PageInventory Test"},
              _init,
              _updateRender,
              [&]() { elements.clear(); });
  LOG(INFO) << "End PageInventory test" << LOG_ENDL;
  return 0;
}

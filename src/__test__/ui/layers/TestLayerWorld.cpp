#include "../../setupTestUi.h"
#include "db/Database.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerWorld.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "model/instances/CharacterPlayer.h"
#include "state/DatabaseInterface.h"
#include "state/LayerManagerInterface.h"
#include "state/StateManagerInterface.h"
#include "state/actions/world/WorldLoadMap.hpp"
#include "state/actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include <memory>
#include <vector>

namespace {

constexpr const char* TEST_PARTY_TEMPLATE_NAMES[] = {
    "testPartyMember1",
    "testPartyMember2",
    "testPartyMember3",
    "testPartyMember4",
    "testPartyMember5",
    "testPartyMember6",
};

const bmin::DynArray<bmin::DynArray<bmin::String>> PARTY_MEMBER_ITEMS = {
    {"PotionHealing", "DaggerBronze"},
    {"ShortSwordBronze", "SwordBronze"},
    {
        "LongbowOak",
        "ArrowsStone",
        "PotionHealing",
    },
    {"ShirtSimple0", "PantsSimple0"},
    {"GlovesLeather", "HatLeather"},
    {"BootsLeather", "NecklaceSilver", "DaggerBronze"},
};

void setupTestParty(model::Player& player, db::Database& database) {
  player.party.clear();
  player.currentPartyMemberIndex = 0;

  for (size_t i = 0; i < PARTY_MEMBER_ITEMS.size(); ++i) {
    model::CharacterPlayer member(
        database.getCharacterTemplate(TEST_PARTY_TEMPLATE_NAMES[i]));

    for (const auto& itemName : PARTY_MEMBER_ITEMS[i]) {
      model::characterPlayerAddItemToInventory(
          member, database.getItemTemplate(bmin::toStringView(itemName)), 1);
    }

    player.party.pushBack(std::move(member));
  }
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start LayerWorld integration test" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  setupTestParty(stateManager.getState().player, database);

  // Load map, spawn player at MarkerPlayer. Default CameraMode::Follow
  // auto-resolves the party avatar; WorldUpdater + LayerWorld view dims snap cam.
  {
    auto& state = stateManager.getState();
    auto loadMap = state::actions::WorldLoadMap("alinea_outsideAlinea1");
    loadMap.execute(&state);
    auto spawnPlayer = state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer");
    spawnPlayer.execute(&state);
  }

  bmin::UniquePtr<layers::LayerManager> layerManager;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "LayerWorld test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);
    state::LayerManagerInterface::setLayerManager(layerManager.get());

    auto* layerWorld = new layers::LayerWorld(&window);
    layerManager->addLayer(layerWorld);

    auto& events = window.getEvents();
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_DOWN,
        [&](int x, int y, int button) { layerManager->handleMouseDown(x, y, button); });
    events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP, [&](int x, int y, int button) {
      layerManager->handleMouseUp(x, y, button);
    });
    events.setMouseEvent(
        sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
        [&](int x, int y, int delta) { layerManager->handleMouseWheel(x, y, delta); });
    events.setKeyboardEvent(
        sdl2w::KeyboardEventCb::ON_KEY_DOWN,
        [&](std::string_view key, int keyCode) {
          layerManager->handleKeyDown(key, keyCode);
        });
    events.setKeyboardEvent(
        sdl2w::KeyboardEventCb::ON_KEY_UP,
        [&](std::string_view key, int keyCode) {
          layerManager->handleKeyUp(key, keyCode);
        });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    layerManager->update(window.getDeltaTime());
    stateManager.update(window.getDeltaTime());

    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "LayerWorld Test"},
              _init,
              _updateRender,
              [&]() { layerManager.reset(); });

  LOG(INFO) << "End LayerWorld integration test" << LOG_ENDL;
  return 0;
}

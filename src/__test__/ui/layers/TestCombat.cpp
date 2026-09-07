#include "../../setupTestUi.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include "db/Database.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/CharacterConstruction.h"
#include "game/map/MapPersistence.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerWorld.h"
#include "model/instances/CharacterInstance.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/RuneTypes.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"
#include "state/WorldUpdater.h"
#include "actions/combat/StartCombat.hpp"
#include "actions/world/WorldLoadActiveMap.hpp"
#include "actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "ui/SdlPixels.h" // IWYU pragma: keep

namespace {

constexpr int kMaxEnemyMarkers = 7;

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

    member.knownSpells.pushBack("FLAME");
    member.equippedRunes = {model::RuneType::HEAT};
    if (member.currentMp < 10) {
      member.currentMp = 10;
    }

    player.party.pushBack(std::move(member));
  }
}

game::ActiveMapMarker findMarkerOnActiveGrid(game::ActiveMapOrchestrator& orch,
                                             const bmin::String& markerName) {
  const auto& grid = orch.getMapGrid();
  for (int y = 0; y < grid.gridHeight; ++y) {
    for (int x = 0; x < grid.gridWidth; ++x) {
      const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
      if (mapName.empty()) {
        continue;
      }
      auto found = orch.findMarker(mapName, markerName);
      if (found.valid) {
        return found;
      }
    }
  }
  return game::ActiveMapMarker{};
}

// Places enemies at Marker Enemy0..EnemyN for each entry in enemyTemplateNames
// (capped at Enemy0..Enemy6).
void spawnEnemiesAtMarkers(state::State& state,
                           db::Database& database,
                           const bmin::DynArray<bmin::String>& enemyTemplateNames) {
  auto& world = state.world;
  if (world.activeMap.gridId.empty()) {
    LOG(ERROR) << "spawnEnemiesAtMarkers: no active map loaded" << LOG_ENDL;
    return;
  }

  game::ActiveMapOrchestrator orch(world.activeMap, state.mapInstances, &database);
  orch.fetchMapGrid(world.activeMap.gridId);

  auto count = enemyTemplateNames.size();
  if (count > static_cast<size_t>(kMaxEnemyMarkers)) {
    count = static_cast<size_t>(kMaxEnemyMarkers);
  }

  for (size_t i = 0; i < count; ++i) {
    const auto markerName = "Enemy" + bmin::toString(static_cast<int>(i));
    const auto found = findMarkerOnActiveGrid(orch, markerName);
    if (!found.valid) {
      LOG(ERROR) << "spawnEnemiesAtMarkers: marker not found: " << markerName << LOG_ENDL;
      continue;
    }

    auto enemy = model::CharacterInstance{};
    enemy.id = "enemy-" + bmin::toString(static_cast<int>(i));
    enemy.templateName = enemyTemplateNames[i];
    enemy.x = found.x;
    enemy.y = found.y;
    enemy.spawnX = found.x;
    enemy.spawnY = found.y;
    game::applyCharacterTemplateFromDatabase(enemy, database);
    world.activeMap.characters.pushBack(std::move(enemy));
  }
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start Combat integration test" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  setupTestParty(stateManager.getState().player, database);

  {
    auto& state = stateManager.getState();
    state.mapInstances = game::createMapInstances(database);

    auto loadMap = state::actions::WorldLoadActiveMap("combat_test1");
    loadMap.execute(&state);
    auto spawnPlayer = state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer");
    spawnPlayer.execute(&state);

    const bmin::DynArray<bmin::String> enemyTemplates = {"goblinTest"};
    spawnEnemiesAtMarkers(state, database, enemyTemplates);

    // Enqueue so SetActiveCombatCharacter (inserted by StartCombat) runs via update.
    stateManager.enqueueAction(
        stateManager.getActionData(), new state::actions::StartCombat(), 0);
    stateManager.update(1);
    state::worldUpdate(nullptr, stateManager, 1);
  }

  bmin::UniquePtr<layers::LayerManager> layerManager;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& store) {
    LOG(INFO) << "Combat test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);

    auto* layerWorld = new layers::LayerWorld(&window);
    layerWorld->setMapScale(2.f);
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
    events.setKeyboardEvent(sdl2w::KeyboardEventCb::ON_KEY_DOWN,
                            [&](std::string_view key, int keyCode) {
                              layerManager->handleKeyDown(key, keyCode);
                            });
    events.setKeyboardEvent(sdl2w::KeyboardEventCb::ON_KEY_UP,
                            [&](std::string_view key, int keyCode) {
                              layerManager->handleKeyUp(key, keyCode);
                            });
  };

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& store) {
    stateManager.update(window.getDeltaTime());
    layerManager->update(window.getDeltaTime());

    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
    return true;
  };

  setupTestUi(
      argc, argv, TestUiParams{800, 600, "Combat Test"}, _init, _updateRender, [&]() {
        layerManager.reset();
      });

  LOG(INFO) << "End Combat integration test" << LOG_ENDL;
  return 0;
}

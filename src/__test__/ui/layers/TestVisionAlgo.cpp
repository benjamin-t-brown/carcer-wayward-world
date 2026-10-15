#include "../../setupTestUi.hpp"
#include "actions/world/WorldLoadActiveMap.hpp"
#include "actions/world/WorldSpawnPlayerAtMarker.hpp"
#include "bmin/UniquePtr.h"
#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerWorld.h"
#include "model/instances/CharacterPlayer.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"
#include "ui/SdlPixels.hpp" // IWYU pragma: keep

namespace {

void setupSinglePlayer(model::Player& player, db::Database& database) {
  player.party.clear();
  player.currentPartyMemberIndex = 0;
  player.party.pushBack(
      model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1")));
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Start VisionAlgo integration test" << LOG_ENDL;

  db::Database database;
  state::DatabaseInterface::setDatabase(&database);
  database.load();

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  setupSinglePlayer(stateManager.getState().player, database);

  {
    auto& state = stateManager.getState();
    state.mapInstances = game::createMapInstances(database);

    constexpr const char* mapName = "visionTest";
    auto loadMap = state::actions::WorldLoadActiveMap(mapName);
    loadMap.execute(&state);
    auto spawnPlayer =
        state::actions::WorldSpawnPlayerAtMarker("MarkerPlayer", mapName);
    spawnPlayer.execute(&state);
  }

  bmin::UniquePtr<layers::LayerManager> layerManager;

  auto _init = [&](sdl2w::Window& window, sdl2w::Store& /*store*/) {
    LOG(INFO) << "VisionAlgo test initialized" << LOG_ENDL;

    layerManager = bmin::makeUnique<layers::LayerManager>(&window);

    auto* layerWorld = new layers::LayerWorld(&window);
    layerWorld->setMapScale(2.f);
    layerManager->addLayer(bmin::UniquePtr<layers::Layer>(layerWorld));

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

  auto _updateRender = [&](sdl2w::Window& window, sdl2w::Store& /*store*/) {
    stateManager.update(window.getDeltaTime());
    layerManager->update(window.getDeltaTime());

    auto& draw = window.getDraw();
    draw.setBackgroundColor(SDL_Color{100, 100, 100, 255});
    draw.clearScreen();
    layerManager->render(window.getDeltaTime());
    return true;
  };

  setupTestUi(argc,
              argv,
              TestUiParams{800, 600, "VisionAlgo Test"},
              _init,
              _updateRender,
              [&]() { layerManager.reset(); });

  LOG(INFO) << "End VisionAlgo integration test" << LOG_ENDL;
  return 0;
}

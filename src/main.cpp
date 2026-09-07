#include "lib/hiscore/hiscore.h"
#include "db/Database.h"
#include "layers/LayerManager.h"
#include "sdl2w/AssetLoader.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Init.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/LayerRequest.h"
#include "state/StateManager.h"

void runProgram(int argc, char** argv) {
  const int w = 640;
  const int h = 480;

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::GPU,
                           .title = "Carcer",
                           .w = w,
                           .h = h,
                           .x = 25, // SDL_WINDOWPOS_UNDEFINED
                           .y = 50, // SDL_WINDOWPOS_UNDEFINED
                           .renderW = w,
                           .renderH = h,
                       });
  sdl2w::L10n::init({"en", "la"});
  sdl2w::setupStartupArgs(argc, argv, window);
  sdl2w::L10n::setLanguage(DISABLE_TRANSLATIONS);
  window.getDraw().setBackgroundColor({0, 0, 0});

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  window.getStore().loadAndStoreFont("default", "assets/cabal.ttf");
  window.getStore().loadAndStoreFont("alternate", "assets/monofonto.ttf");

  window.setSoundPct(33);

  db::Database database;
  database.load();
  state::DatabaseInterface::setDatabase(&database);
  state::StateManager stateManager;
  state::pushLayerRequest(stateManager.getState(),
                          state::LayerRequest{.id = state::LayerId::World});
  layers::LayerManager layerManager(&window);
  layerManager.start();
  state::DatabaseInterface::setDatabase(nullptr);
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  runProgram(argc, argv);

  sdl2w::Window::unInit();
  LOG(INFO) << "End program" << LOG_ENDL;

  return 0;
}

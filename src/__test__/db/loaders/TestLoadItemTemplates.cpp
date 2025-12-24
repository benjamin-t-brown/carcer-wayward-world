#include "db/loaders/LoadItemTemplates.h"
#include "lib/sdl2w/Logger.h"
#include "model/Items.h"
#include <unordered_map>

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadItemTemplates" << LOG_ENDL;

  std::unordered_map<std::string, model::ItemTemplate> itemTemplates;

  try {
    db::loadItemTemplates("assets/db/items.json", itemTemplates);
    LOG(INFO) << "Successfully loaded " << itemTemplates.size() << " item templates"
              << LOG_ENDL;

    LOG(INFO) << "TestLoadItemTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading item templates: " << e.what() << LOG_ENDL;
    return 1;
  }
}
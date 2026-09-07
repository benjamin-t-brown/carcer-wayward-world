#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
import carcer.db;
import sdl2w;
import bmin.string_interop;
#include "macros.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadItemTemplates" << LOG_ENDL;

  bmin::Map<bmin::String, model::ItemTemplate> itemTemplates;

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

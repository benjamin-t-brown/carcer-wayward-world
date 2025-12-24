#include "db/loaders/LoadSpecialEvents.h"
#include "lib/sdl2w/Logger.h"
#include "model/SpecialEvents.h"
#include <unordered_map>
#include <vector>

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadSpecialEvents" << LOG_ENDL;

  std::unordered_map<std::string, model::GameEvent> specialEvents;

  try {
    db::loadSpecialEvents("assets/db/special-events.json", specialEvents);
    LOG(INFO) << "Successfully loaded " << specialEvents.size() << " special events"
              << LOG_ENDL;

    // Test filtering by eventsToLoad
    std::unordered_map<std::string, model::GameEvent> filteredEvents;
    std::vector<std::string> eventsToLoad = {"alinea_claire"};
    db::loadSpecialEvents("assets/db/special-events.json", filteredEvents, eventsToLoad);
    LOG(INFO) << "Successfully loaded " << filteredEvents.size()
              << " filtered special events" << LOG_ENDL;

    if (filteredEvents.size() != 1) {
      LOG(ERROR) << "Expected 1 filtered event, got " << filteredEvents.size() << LOG_ENDL;
      return 1;
    }

    if (filteredEvents.find("alinea_claire") == filteredEvents.end()) {
      LOG(ERROR) << "Expected event 'alinea_claire' not found in filtered events" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadSpecialEvents completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading special events: " << e.what() << LOG_ENDL;
    return 1;
  }
}


#include "db/loaders/LoadSpecialEvents.h"
#include "lib/sdl2w/Logger.h"
#include <unordered_map>

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadSpecialEvents" << LOG_ENDL;

  std::unordered_map<std::string, model::GameEvent> gameEvents;

  try {
    db::loadSpecialEvents("assets/db/events-test.se", gameEvents);
    LOG(INFO) << "Successfully loaded " << gameEvents.size() << " game events" << LOG_ENDL;

    // Print some information about loaded events
    for (const auto& [eventId, event] : gameEvents) {
      LOG(INFO) << "Event ID: " << eventId << ", Title: " << event.title
                << ", Icon: " << event.icon << ", Children: " << event.children.size()
                << ", Vars: " << event.vars.size() << LOG_ENDL;
    }

    LOG(INFO) << "TestLoadSpecialEvents completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading special events: " << e.what() << LOG_ENDL;
    return 1;
  }
}


#pragma once

#include "model/SpecialEvents.h"
#include <string>
#include <unordered_map>

namespace db {

void loadSpecialEvents(const std::string& specialEventsFilePath,
                       std::unordered_map<std::string, model::GameEvent>& specialEvents);

void loadSpecialEvents(const std::string& specialEventsFilePath,
                       std::unordered_map<std::string, model::GameEvent>& specialEvents,
                       std::vector<std::string>& eventsToLoad);

} // namespace db

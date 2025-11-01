#pragma once

#include "model/SpecialEvents.h"
#include <string>
#include <unordered_map>

namespace db {

void loadSpecialEvents(
    const std::string& eventsFilePath,
    std::unordered_map<std::string, model::GameEvent>& gameEvents);

} // namespace db


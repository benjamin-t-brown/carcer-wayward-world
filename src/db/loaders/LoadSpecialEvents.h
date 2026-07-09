#pragma once

#include "lib/Types.h"
#include "lib/bmin/Map.h"
#include "model/templates/SpecialEvents.h"

namespace db {

void loadSpecialEvents(const String& specialEventsFilePath,
                       bmin::Map<String, model::GameEvent>& specialEvents);

void loadSpecialEvents(const String& specialEventsFilePath,
                       bmin::Map<String, model::GameEvent>& specialEvents,
                       bmin::DynArray<String>& eventsToLoad);

} // namespace db

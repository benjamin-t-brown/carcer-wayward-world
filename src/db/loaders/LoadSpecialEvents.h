#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "lib/bmin/Map.h"
#include "model/templates/SpecialEvents.h"

namespace db {

void loadSpecialEvents(const bmin::String& specialEventsFilePath,
                       bmin::Map<bmin::String, model::GameEvent>& specialEvents);

void loadSpecialEvents(const bmin::String& specialEventsFilePath,
                       bmin::Map<bmin::String, model::GameEvent>& specialEvents,
                       bmin::DynArray<bmin::String>& eventsToLoad);

} // namespace db

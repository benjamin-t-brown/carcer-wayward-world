#pragma once

#include "bmin/String.h"
#include "bmin/Map.h"
#include "model/templates/Quests.hpp"

namespace db {

void loadQuestTemplates(const bmin::String& questsFilePath,
                        bmin::Map<bmin::String, model::QuestTemplate>& questTemplates);

} // namespace db

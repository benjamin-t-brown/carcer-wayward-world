#pragma once

#include "bmin/Map.h"
#include "bmin/String.h"
#include "model/templates/Quests.hpp"

namespace in3 {

inline constexpr const char* kQuestCompleteStepId = "complete";

void setQuestTemplates(const bmin::Map<bmin::String, model::QuestTemplate>* questTemplates);
const model::QuestTemplate* findQuestTemplate(const bmin::String& questId);

bmin::String questStepStorageKey(const bmin::String& questName);
bmin::String questCompletedStepStorageKey(const bmin::String& questName,
                                          const bmin::String& stepId);

void startQuest(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& questName);
void completeQuestStep(bmin::Map<bmin::String, bmin::String>& storage,
                       const bmin::String& questName, const bmin::String& stepId);
void completeQuest(bmin::Map<bmin::String, bmin::String>& storage,
                   const bmin::String& questName);

bool questIsStarted(const bmin::Map<bmin::String, bmin::String>& storage,
                    const bmin::String& questName);
bool questIsComplete(const bmin::Map<bmin::String, bmin::String>& storage,
                     const bmin::String& questName);
bool questStepEq(const bmin::Map<bmin::String, bmin::String>& storage,
                 const bmin::String& questName, const bmin::String& stepId);

} // namespace in3

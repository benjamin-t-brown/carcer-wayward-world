#pragma once

#include "bmin/DynArray.h"
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
bmin::String questShownStepStorageKey(const bmin::String& questName,
                                      const bmin::String& stepId);

void startQuest(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& questName);
void setQuestStepEq(bmin::Map<bmin::String, bmin::String>& storage,
                    const bmin::String& questName, const bmin::String& stepId);
void completeQuestStep(bmin::Map<bmin::String, bmin::String>& storage,
                       const bmin::String& questName, const bmin::String& stepId);
void showQuestSubStep(bmin::Map<bmin::String, bmin::String>& storage,
                      const bmin::String& questName, const bmin::String& stepId,
                      const bmin::String& subStepId);
void hideQuestSubStep(bmin::Map<bmin::String, bmin::String>& storage,
                      const bmin::String& questName, const bmin::String& stepId,
                      const bmin::String& subStepId);
void completeQuestSubStep(bmin::Map<bmin::String, bmin::String>& storage,
                          const bmin::String& questName, const bmin::String& stepId,
                          const bmin::String& subStepId);
void completeQuest(bmin::Map<bmin::String, bmin::String>& storage,
                   const bmin::String& questName);

bool questIsStarted(const bmin::Map<bmin::String, bmin::String>& storage,
                    const bmin::String& questName);
bool questIsComplete(const bmin::Map<bmin::String, bmin::String>& storage,
                     const bmin::String& questName);
bool questStepEq(const bmin::Map<bmin::String, bmin::String>& storage,
                 const bmin::String& questName, const bmin::String& stepId);
bool questStepIsCompleted(const bmin::Map<bmin::String, bmin::String>& storage,
                          const bmin::String& questName, const bmin::String& stepId);
bool questSubStepIsShown(const bmin::Map<bmin::String, bmin::String>& storage,
                         const bmin::String& questName, const bmin::String& stepId,
                         const bmin::String& subStepId);
bool questStepVisibleInJournal(const bmin::Map<bmin::String, bmin::String>& storage,
                               const bmin::String& questName, const bmin::String& stepId);
bmin::DynArray<bmin::String> questJournalVisibleStepIds(
    const bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& questName);

} // namespace in3

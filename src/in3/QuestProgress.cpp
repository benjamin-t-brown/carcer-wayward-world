#include "QuestProgress.h"
#include "EventRunnerHelpers.h"

#include "bmin/DynArray.h"

#include <optional>

namespace in3 {
namespace {

const bmin::Map<bmin::String, model::QuestTemplate>* gQuestTemplates = nullptr;

bool isTruthyCompleted(const std::optional<bmin::String>& value) {
  return value && !value->empty() && *value != "0" && *value != "false";
}

bool isNestedSubStepOf(const model::QuestTemplate& quest,
                       const bmin::String& stepId,
                       const bmin::String& subStepId) {
  for (size_t i = 0; i < quest.steps.size(); ++i) {
    if (quest.steps[i].id != stepId) {
      continue;
    }
    const auto& subSteps = quest.steps[i].subSteps;
    for (size_t j = 0; j < subSteps.size(); ++j) {
      if (subSteps[j].id == subStepId) {
        return true;
      }
    }
    return false;
  }
  return false;
}

bool isNestedSubStep(const model::QuestTemplate& quest, const bmin::String& stepId) {
  for (size_t i = 0; i < quest.steps.size(); ++i) {
    if (isNestedSubStepOf(quest, quest.steps[i].id, stepId)) {
      return true;
    }
  }
  return false;
}

void clearPrefixedKeys(bmin::Map<bmin::String, bmin::String>& storage,
                       const bmin::String& prefix) {
  bmin::DynArray<bmin::String> keysToErase;
  for (auto it = storage.begin(); it != storage.end(); ++it) {
    if (it->key.startsWith(prefix.cStr())) {
      keysToErase.pushBack(it->key);
    }
  }
  for (const auto& key : keysToErase) {
    storage.erase(key);
  }
}

void clearQuestRuntimeFlags(bmin::Map<bmin::String, bmin::String>& storage,
                            const bmin::String& questName) {
  const bmin::String questPrefix = bmin::String("vars.quests.") + questName + ".";
  clearPrefixedKeys(storage, questPrefix + "completed.");
  clearPrefixedKeys(storage, questPrefix + "shown.");
}

bool nestedSubStepAllowed(const bmin::String& questName, const bmin::String& stepId,
                          const bmin::String& subStepId) {
  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (quest && !isNestedSubStepOf(*quest, stepId, subStepId)) {
    return false;
  }
  return true;
}

} // namespace

void setQuestTemplates(const bmin::Map<bmin::String, model::QuestTemplate>* questTemplates) {
  gQuestTemplates = questTemplates;
}

const model::QuestTemplate* findQuestTemplate(const bmin::String& questId) {
  if (!gQuestTemplates) {
    return nullptr;
  }
  auto it = gQuestTemplates->find(questId);
  if (it == gQuestTemplates->end()) {
    return nullptr;
  }
  return &(*it).value;
}

bmin::String questStepStorageKey(const bmin::String& questName) {
  return bmin::String("vars.quests.") + questName + ".step";
}

bmin::String questCompletedStepStorageKey(const bmin::String& questName,
                                          const bmin::String& stepId) {
  return bmin::String("vars.quests.") + questName + ".completed." + stepId;
}

bmin::String questShownStepStorageKey(const bmin::String& questName,
                                      const bmin::String& stepId) {
  return bmin::String("vars.quests.") + questName + ".shown." + stepId;
}

void startQuest(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& questName) {
  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (!quest || quest->steps.empty()) {
    return;
  }
  clearQuestRuntimeFlags(storage, questName);
  setStorage(storage, questStepStorageKey(questName), quest->steps[0].id);
}

void setQuestStepEq(bmin::Map<bmin::String, bmin::String>& storage,
                    const bmin::String& questName, const bmin::String& stepId) {
  setStorage(storage, questStepStorageKey(questName), stepId);
}

void completeQuestStep(bmin::Map<bmin::String, bmin::String>& storage,
                       const bmin::String& questName, const bmin::String& stepId) {
  setStorage(storage, questCompletedStepStorageKey(questName, stepId), "true");
}

void showQuestSubStep(bmin::Map<bmin::String, bmin::String>& storage,
                      const bmin::String& questName, const bmin::String& stepId,
                      const bmin::String& subStepId) {
  if (!nestedSubStepAllowed(questName, stepId, subStepId)) {
    return;
  }
  setStorage(storage, questShownStepStorageKey(questName, subStepId), "true");
}

void hideQuestSubStep(bmin::Map<bmin::String, bmin::String>& storage,
                      const bmin::String& questName, const bmin::String& stepId,
                      const bmin::String& subStepId) {
  if (!nestedSubStepAllowed(questName, stepId, subStepId)) {
    return;
  }
  storage.erase(questShownStepStorageKey(questName, subStepId));
}

void completeQuestSubStep(bmin::Map<bmin::String, bmin::String>& storage,
                          const bmin::String& questName, const bmin::String& stepId,
                          const bmin::String& subStepId) {
  if (!nestedSubStepAllowed(questName, stepId, subStepId)) {
    return;
  }
  setStorage(storage, questCompletedStepStorageKey(questName, subStepId), "true");
}

void completeQuest(bmin::Map<bmin::String, bmin::String>& storage,
                   const bmin::String& questName) {
  setStorage(storage, questStepStorageKey(questName), kQuestCompleteStepId);
}

bool questIsComplete(const bmin::Map<bmin::String, bmin::String>& storage,
                     const bmin::String& questName) {
  auto step = getStorage(storage, questStepStorageKey(questName));
  return step && *step == kQuestCompleteStepId;
}

bool questIsStarted(const bmin::Map<bmin::String, bmin::String>& storage,
                    const bmin::String& questName) {
  auto step = getStorage(storage, questStepStorageKey(questName));
  return step && !step->empty() && *step != kQuestCompleteStepId;
}

bool questStepEq(const bmin::Map<bmin::String, bmin::String>& storage,
                 const bmin::String& questName, const bmin::String& stepId) {
  auto current = getStorage(storage, questStepStorageKey(questName));
  if (current && *current == stepId) {
    return true;
  }
  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (!quest || !isNestedSubStep(*quest, stepId)) {
    return false;
  }
  return isTruthyCompleted(getStorage(storage, questCompletedStepStorageKey(questName, stepId)));
}

bool questStepIsCompleted(const bmin::Map<bmin::String, bmin::String>& storage,
                          const bmin::String& questName, const bmin::String& stepId) {
  return isTruthyCompleted(
      getStorage(storage, questCompletedStepStorageKey(questName, stepId)));
}

bool questSubStepIsShown(const bmin::Map<bmin::String, bmin::String>& storage,
                         const bmin::String& questName, const bmin::String& stepId,
                         const bmin::String& subStepId) {
  if (!nestedSubStepAllowed(questName, stepId, subStepId)) {
    return false;
  }
  return isTruthyCompleted(getStorage(storage, questShownStepStorageKey(questName, subStepId)));
}

bool questStepVisibleInJournal(const bmin::Map<bmin::String, bmin::String>& storage,
                               const bmin::String& questName, const bmin::String& stepId) {
  if (questStepIsCompleted(storage, questName, stepId)) {
    return true;
  }
  auto current = getStorage(storage, questStepStorageKey(questName));
  if (current && *current == stepId) {
    return true;
  }
  return isTruthyCompleted(getStorage(storage, questShownStepStorageKey(questName, stepId)));
}

bmin::DynArray<bmin::String> questJournalVisibleStepIds(
    const bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& questName) {
  bmin::DynArray<bmin::String> visible;
  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (quest) {
    for (size_t i = 0; i < quest->steps.size(); ++i) {
      const auto& step = quest->steps[i];
      if (questStepVisibleInJournal(storage, questName, step.id)) {
        visible.pushBack(step.id);
      }
      for (size_t j = 0; j < step.subSteps.size(); ++j) {
        if (questStepVisibleInJournal(storage, questName, step.subSteps[j].id)) {
          visible.pushBack(step.subSteps[j].id);
        }
      }
    }
    return visible;
  }
  auto current = getStorage(storage, questStepStorageKey(questName));
  if (current && !current->empty() && *current != kQuestCompleteStepId) {
    visible.pushBack(*current);
  }
  return visible;
}

} // namespace in3

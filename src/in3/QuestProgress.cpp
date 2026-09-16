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

int findTopLevelStepIndex(const model::QuestTemplate& quest, const bmin::String& stepId) {
  for (size_t i = 0; i < quest.steps.size(); ++i) {
    if (quest.steps[i].id == stepId) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

bool isNestedSubStep(const model::QuestTemplate& quest, const bmin::String& stepId) {
  for (size_t i = 0; i < quest.steps.size(); ++i) {
    const auto& subSteps = quest.steps[i].subSteps;
    for (size_t j = 0; j < subSteps.size(); ++j) {
      if (subSteps[j].id == stepId) {
        return true;
      }
    }
  }
  return false;
}

void clearCompletedSteps(bmin::Map<bmin::String, bmin::String>& storage,
                         const bmin::String& questName) {
  const bmin::String prefix = bmin::String("vars.quests.") + questName + ".completed.";
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

void startQuest(bmin::Map<bmin::String, bmin::String>& storage, const bmin::String& questName) {
  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (!quest || quest->steps.empty()) {
    return;
  }
  clearCompletedSteps(storage, questName);
  setStorage(storage, questStepStorageKey(questName), quest->steps[0].id);
}

void completeQuestStep(bmin::Map<bmin::String, bmin::String>& storage,
                       const bmin::String& questName, const bmin::String& stepId) {
  setStorage(storage, questCompletedStepStorageKey(questName, stepId), "true");

  const model::QuestTemplate* quest = findQuestTemplate(questName);
  if (!quest) {
    setStorage(storage, questStepStorageKey(questName), stepId);
    return;
  }

  const int topLevelIndex = findTopLevelStepIndex(*quest, stepId);
  if (topLevelIndex < 0) {
    return;
  }
  const size_t nextIndex = static_cast<size_t>(topLevelIndex) + 1;
  if (nextIndex < quest->steps.size()) {
    setStorage(storage, questStepStorageKey(questName), quest->steps[nextIndex].id);
  } else {
    setStorage(storage, questStepStorageKey(questName), stepId);
  }
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

} // namespace in3

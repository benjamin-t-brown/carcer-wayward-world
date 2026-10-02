#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

struct QuestStep {
  bmin::String id;
  bmin::String label;
  bmin::String description;
  bmin::DynArray<QuestStep> subSteps;
};

struct QuestRewardItem {
  bmin::String name;
  int amount = 1;
};

struct QuestRewards {
  int coins = 0;
  int experience = 0;
  bmin::DynArray<QuestRewardItem> items;
};

struct QuestTemplate {
  bmin::String id;
  bmin::String label;
  bmin::String description;
  bmin::String completedDescription;
  bmin::DynArray<QuestStep> steps;
  QuestRewards rewards;
};

} // namespace model

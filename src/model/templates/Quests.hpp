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

struct QuestTemplate {
  bmin::String id;
  bmin::String label;
  bmin::String description;
  bmin::String completedDescription;
  bmin::DynArray<QuestStep> steps;
};

} // namespace model

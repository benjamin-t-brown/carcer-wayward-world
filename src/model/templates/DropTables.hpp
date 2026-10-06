#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

struct DropTableEntry {
  bmin::String item;
  bmin::String dropTable;
  int weight = 1;
  int goldMin = 0;
  int goldMax = 0;
  int foodMin = 0;
  int foodMax = 0;
  bool isGoldRangeEntry = false;
  bool isFoodRangeEntry = false;
  bool isNothingEntry = false;
};

struct DropTableTemplate {
  bmin::String name;
  bmin::String label;
  bmin::DynArray<DropTableEntry> entries;
};

} // namespace model

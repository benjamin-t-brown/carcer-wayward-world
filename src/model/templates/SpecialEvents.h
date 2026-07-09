#pragma once

#include <optional>
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include <variant>

namespace model {

enum class GameEventType { MODAL, TALK };
enum class GameEventChildType { KEYWORD, CHOICE, END, EXEC, SWITCH };
// enum class KeywordType { K, K_DUP, K_SWITCH, K_CHILD };

struct VariableValue;

// struct GameEventChildKeyword;
struct GameEventChildChoice;
struct GameEventChildSwitch;
struct GameEventChildExec;
struct GameEventChildEnd;
struct Variable;

// Discriminated union for GameEventChild
using GameEventChild = std::variant<GameEventChildChoice,
                                    GameEventChildSwitch,
                                    GameEventChildExec,
                                    GameEventChildEnd>;

struct GameEvent {
  bmin::String id;
  bmin::String title;
  GameEventType eventType; // indicates which ui layer to use for the event
  bmin::String icon;        // name of sprite to use for the event
  // a mapping from variable name to its original text and it's evaluated value
  bmin::DynArray<Variable> vars;
  bmin::DynArray<GameEventChild> children;
};

struct Variable {
  bmin::String id;
  bmin::String key;
  bmin::String value;
  bmin::String importFrom;
};

struct VariableValue {
  bmin::String str;
  std::optional<bmin::String> evaluated;
};

struct AudioInfo {
  bmin::String audioName;
  int volume;
  int offset;
};

// struct KeywordDataK {
//   KeywordType keywordType = KeywordType::K;
//   bmin::String text;
// };

// struct KeywordDataKDup {
//   KeywordType keywordType = KeywordType::K_DUP;
//   bmin::String keyword;
// };

// struct KeywordCheck {
//   bmin::String conditionStr;
//   bmin::String next;
// };

// struct KeywordDataKSwitch {
//   KeywordType keywordType = KeywordType::K_SWITCH;
//   bmin::String defaultNext; // id of next child
//   bmin::DynArray<KeywordCheck> checks;
// };

// struct KeywordDataKChild {
//   KeywordType keywordType = KeywordType::K_CHILD;
//   bmin::String next; // id of next child
// };

// // Discriminated union for KeywordData
// struct KeywordData {
//   KeywordType keywordType;
//   // Union of possible data - only one should be populated based on keywordType
//   std::optional<KeywordDataK> k;
//   std::optional<KeywordDataKDup> kDup;
//   std::optional<KeywordDataKSwitch> kSwitch;
//   std::optional<KeywordDataKChild> kChild;
// };

// struct GameEventChildKeyword {
//   GameEventChildType eventChildType = GameEventChildType::KEYWORD;
//   bmin::String id;
//   std::map<bmin::String, KeywordData> keywords;
// };

struct ChoiceSwitchText {
  bmin::String conditionStr;
  bmin::String text;
};

struct Choice {
  bmin::String text;
  bmin::DynArray<ChoiceSwitchText> switchText;
  bmin::String prefixText;
  bmin::String conditionStr;
  bmin::String evalStr;
  bmin::String next;
};



struct GameEventChildChoice {
  GameEventChildType eventChildType = GameEventChildType::CHOICE;
  bmin::String id;
  bmin::String text;
  bmin::DynArray<Choice> choices;
  std::optional<AudioInfo> audioInfo;
};

struct SwitchCase {
  bmin::String conditionStr;
  bmin::String next;
};

struct GameEventChildSwitch {
  GameEventChildType eventChildType = GameEventChildType::SWITCH;
  bmin::String id;
  bmin::String defaultNext;
  bmin::DynArray<SwitchCase> cases;
};

struct GameEventChildExec {
  GameEventChildType eventChildType = GameEventChildType::EXEC;
  bmin::String id;
  bmin::DynArray<bmin::String> paragraphs;
  bmin::String execStr;
  bmin::String next;
  bool autoAdvance;
  std::optional<AudioInfo> audioInfo;
};

struct GameEventChildEnd {
  GameEventChildType eventChildType = GameEventChildType::END;
  bmin::String id;
  bmin::String next;
};

} // namespace model

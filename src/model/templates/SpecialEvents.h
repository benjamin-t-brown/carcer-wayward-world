#pragma once

#include <optional>
#include "lib/Types.h"
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
  String id;
  String title;
  GameEventType eventType; // indicates which ui layer to use for the event
  String icon;        // name of sprite to use for the event
  // a mapping from variable name to its original text and it's evaluated value
  bmin::DynArray<Variable> vars;
  bmin::DynArray<GameEventChild> children;
};

struct Variable {
  String id;
  String key;
  String value;
  String importFrom;
};

struct VariableValue {
  String str;
  std::optional<String> evaluated;
};

struct AudioInfo {
  String audioName;
  int volume;
  int offset;
};

// struct KeywordDataK {
//   KeywordType keywordType = KeywordType::K;
//   String text;
// };

// struct KeywordDataKDup {
//   KeywordType keywordType = KeywordType::K_DUP;
//   String keyword;
// };

// struct KeywordCheck {
//   String conditionStr;
//   String next;
// };

// struct KeywordDataKSwitch {
//   KeywordType keywordType = KeywordType::K_SWITCH;
//   String defaultNext; // id of next child
//   bmin::DynArray<KeywordCheck> checks;
// };

// struct KeywordDataKChild {
//   KeywordType keywordType = KeywordType::K_CHILD;
//   String next; // id of next child
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
//   String id;
//   std::map<String, KeywordData> keywords;
// };

struct ChoiceSwitchText {
  String conditionStr;
  String text;
};

struct Choice {
  String text;
  bmin::DynArray<ChoiceSwitchText> switchText;
  String prefixText;
  String conditionStr;
  String evalStr;
  String next;
};



struct GameEventChildChoice {
  GameEventChildType eventChildType = GameEventChildType::CHOICE;
  String id;
  String text;
  bmin::DynArray<Choice> choices;
  std::optional<AudioInfo> audioInfo;
};

struct SwitchCase {
  String conditionStr;
  String next;
};

struct GameEventChildSwitch {
  GameEventChildType eventChildType = GameEventChildType::SWITCH;
  String id;
  String defaultNext;
  bmin::DynArray<SwitchCase> cases;
};

struct GameEventChildExec {
  GameEventChildType eventChildType = GameEventChildType::EXEC;
  String id;
  bmin::DynArray<String> paragraphs;
  String execStr;
  String next;
  bool autoAdvance;
  std::optional<AudioInfo> audioInfo;
};

struct GameEventChildEnd {
  GameEventChildType eventChildType = GameEventChildType::END;
  String id;
  String next;
};

} // namespace model

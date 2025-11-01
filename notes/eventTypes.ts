export interface GameEvent {
  title: string;
  eventType: 'modal' | 'talk';  // indicates which ui layer to use for the event
  icon: string; // name of sprite to use for the event
  vars: Record<string, { str: string; evaluated?: string }>; // a mapping from variable name to its original text and it's evaluated value
  children: GameEventChild[];
}

export interface GameEventChild {
  id: string;
  eventChildType: 'keyword' | 'choice' | 'end' | 'exec' | 'bool' | 'switch';
}

export interface KeywordData {
  keywordType: 'k' | 'kDup' | 'kSwitch' | 'kChild';
}

export interface KeywordDataK extends KeywordData {
  keywordType: 'k';
  text: string;
}

export interface KeywordDataKDup extends KeywordData {
  keywordType: 'kDup';
  keyword: string;
}

export interface KeywordDataKSwitch extends KeywordData {
  keywordType: 'kSwitch';
  default: string; // id of next child
  checks: {
    conditionStr: string;
    next: string;
  }[];
}

export interface KeywordDataKChild extends KeywordData {
  keywordType: 'kChild';
  next: string; // id of next child
}

export interface GameEventChildKeyword extends GameEventChild {
  keywords: Record<string, KeywordData>;
}

export interface Choice {
  text: string;
  conditionStr?: string; // string that represents a condition to evaluate
  next: string;
}

export interface GameEventChildChoice extends GameEventChild {
  choiceType: 'choice';
  text: string;
  choices: Choice[];
}

export interface GameEventChildSwitch extends GameEventChild {
  switchType: 'switch';
  switchStr: string;
  cases: {
    conditionStr: string;
    next: string;
  }[];
}

export interface GameEventChildExec extends GameEventChild {
  execType: 'exec';
  text: string;
  execStr: string;
  next: string;
}

export interface GameEventChildBool extends GameEventChild {
  boolType: 'bool';
  checkStr: string;
  pass: string; // id of next child
  fail: string; // id of next child
}

export interface GameEventChildEnd extends GameEventChild {
  endType: 'end';
  next: string;
}

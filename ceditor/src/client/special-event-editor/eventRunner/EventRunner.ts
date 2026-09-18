import {
  GameEvent,
  GameEventChildChoice,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildType,
  Choice,
  QuestTemplate,
  ItemTemplate,
} from '../../types/assets';
import { getVarsFromNode } from '../nodeHelpers';

export const JOURNAL_UPDATED_MESSAGE = 'Your journal has been updated.';

export function itemReceivedNoticeText(itemLabel: string) {
  return `You have received ${itemLabel}.`;
}

export function splitExecStatements(str: string): string[] {
  const statements: string[] = [];
  let parenDepth = 0;
  let current = '';
  for (let i = 0; i < str.length; i++) {
    const c = str[i];
    if (c === '(') {
      parenDepth++;
      current += c;
    } else if (c === ')') {
      parenDepth--;
      current += c;
    } else if ((c === ';' || c === '\n') && parenDepth === 0) {
      const trimmed = current.trim();
      if (trimmed) {
        statements.push(trimmed);
      }
      current = '';
    } else {
      current += c;
    }
  }
  const trimmed = current.trim();
  if (trimmed) {
    statements.push(trimmed);
  }
  return statements;
}

const setStorage = (
  storage: Record<string, string | undefined>,
  key: string,
  value: string | undefined
) => {
  const keys = key.split('.');
  let curr: any = storage;
  for (let i = 0; i < keys.length - 1; i++) {
    if (typeof curr[keys[i]] !== 'object' || curr[keys[i]] === null) {
      curr[keys[i]] = {};
    }
    curr = curr[keys[i]];
  }
  curr[keys[keys.length - 1]] = String(value);
};

const getStorage = (
  storage: Record<string, string | undefined>,
  key: string
): string | undefined => {
  const keys = key.split('.');
  let curr: any = storage;
  for (let i = 0; i < keys.length; i++) {
    if (!curr) {
      return curr;
    }
    const key = keys[i];
    const next: any = curr[key];
    curr = next;
  }
  if (typeof curr === 'object') {
    return '[object]';
  }
  return curr;
};

const QUEST_COMPLETE_STEP_ID = 'complete';

function findQuestTemplate(
  quests: QuestTemplate[],
  questName: string,
): QuestTemplate | undefined {
  return quests.find((quest) => quest.id === questName);
}

function isNestedSubStepOf(
  quest: QuestTemplate,
  stepId: string,
  subStepId: string,
): boolean {
  const parent = (quest.steps ?? []).find((step) => step.id === stepId);
  return Boolean(
    parent && (parent.subSteps ?? []).some((subStep) => subStep.id === subStepId),
  );
}

function isNestedSubStep(quest: QuestTemplate, stepId: string): boolean {
  return (quest.steps ?? []).some((step) =>
    isNestedSubStepOf(quest, step.id, stepId),
  );
}

function clearQuestRuntimeFlags(storage: Record<string, any>, questName: string) {
  const questState = storage?.vars?.quests?.[questName];
  if (questState && typeof questState === 'object') {
    delete questState.completed;
    delete questState.shown;
  }
}

function startQuest(
  storage: Record<string, any>,
  quests: QuestTemplate[],
  questName: string,
) {
  const quest = findQuestTemplate(quests, questName);
  const firstStep = quest?.steps?.[0];
  if (!firstStep?.id) {
    return;
  }
  clearQuestRuntimeFlags(storage, questName);
  setStorage(storage, `vars.quests.${questName}.step`, firstStep.id);
}

function setQuestStepEq(
  storage: Record<string, any>,
  questName: string,
  stepId: string,
) {
  setStorage(storage, `vars.quests.${questName}.step`, stepId);
}

function completeQuestStep(
  storage: Record<string, any>,
  questName: string,
  stepId: string,
) {
  setStorage(storage, `vars.quests.${questName}.completed.${stepId}`, 'true');
}

function nestedSubStepAllowed(
  quests: QuestTemplate[],
  questName: string,
  stepId: string,
  subStepId: string,
) {
  const quest = findQuestTemplate(quests, questName);
  if (quest && !isNestedSubStepOf(quest, stepId, subStepId)) {
    return false;
  }
  return true;
}

function deleteStorage(storage: Record<string, any>, key: string) {
  const keys = key.split('.');
  let curr: any = storage;
  for (let i = 0; i < keys.length - 1; i++) {
    if (typeof curr[keys[i]] !== 'object' || curr[keys[i]] === null) {
      return;
    }
    curr = curr[keys[i]];
  }
  delete curr[keys[keys.length - 1]];
}

function showQuestSubStep(
  storage: Record<string, any>,
  quests: QuestTemplate[],
  questName: string,
  stepId: string,
  subStepId: string,
) {
  if (!nestedSubStepAllowed(quests, questName, stepId, subStepId)) {
    return;
  }
  setStorage(storage, `vars.quests.${questName}.shown.${subStepId}`, 'true');
}

function hideQuestSubStep(
  storage: Record<string, any>,
  quests: QuestTemplate[],
  questName: string,
  stepId: string,
  subStepId: string,
) {
  if (!nestedSubStepAllowed(quests, questName, stepId, subStepId)) {
    return;
  }
  deleteStorage(storage, `vars.quests.${questName}.shown.${subStepId}`);
}

function completeQuestSubStep(
  storage: Record<string, any>,
  quests: QuestTemplate[],
  questName: string,
  stepId: string,
  subStepId: string,
) {
  if (!nestedSubStepAllowed(quests, questName, stepId, subStepId)) {
    return;
  }
  setStorage(storage, `vars.quests.${questName}.completed.${subStepId}`, 'true');
}

function completeQuest(storage: Record<string, any>, questName: string) {
  setStorage(storage, `vars.quests.${questName}.step`, QUEST_COMPLETE_STEP_ID);
}

function questIsStarted(storage: Record<string, any>, questName: string) {
  const step = getStorage(storage, `vars.quests.${questName}.step`);
  return Boolean(step && step !== QUEST_COMPLETE_STEP_ID);
}

function questIsComplete(storage: Record<string, any>, questName: string) {
  return getStorage(storage, `vars.quests.${questName}.step`) === QUEST_COMPLETE_STEP_ID;
}

function isTruthyCompleted(value: any): boolean {
  return Boolean(value && value !== '0' && value !== 'false');
}

function questStepEq(
  storage: Record<string, any>,
  quests: QuestTemplate[],
  questName: string,
  stepId: string,
) {
  if (getStorage(storage, `vars.quests.${questName}.step`) === stepId) {
    return true;
  }
  const quest = findQuestTemplate(quests, questName);
  if (!quest || !isNestedSubStep(quest, stepId)) {
    return false;
  }
  return isTruthyCompleted(
    getStorage(storage, `vars.quests.${questName}.completed.${stepId}`),
  );
}

function questStepIsCompleted(
  storage: Record<string, any>,
  questName: string,
  stepId: string,
) {
  return isTruthyCompleted(
    getStorage(storage, `vars.quests.${questName}.completed.${stepId}`),
  );
}

function questSubStepIsShown(
  storage: Record<string, any>,
  quests: QuestTemplate[],
  questName: string,
  stepId: string,
  subStepId: string,
) {
  if (!nestedSubStepAllowed(quests, questName, stepId, subStepId)) {
    return false;
  }
  return isTruthyCompleted(
    getStorage(storage, `vars.quests.${questName}.shown.${subStepId}`),
  );
}

class ConditionEvaluator {
  storage: Record<string, any>;
  baseConditionStr: string;
  quests: QuestTemplate[];
  onceKeysToCommit: string[] = [];

  constructor(
    storage: Record<string, any>,
    baseConditionStr: string,
    quests: QuestTemplate[] = [],
  ) {
    this.storage = storage;
    this.baseConditionStr = baseConditionStr;
    this.quests = quests;
  }

  parseFunctionCall(str: string) {
    const firstParen = str.indexOf('(');
    const lastParen = str.lastIndexOf(')');
    const funcName = str.substring(0, firstParen);
    const funcArgsStr = str.substring(firstParen + 1, lastParen);

    // Parse arguments while considering inner parens
    const funcArgs = [];
    let parenDepth = 0;
    let currentArg = '';
    for (let i = 0; i < funcArgsStr.length; i++) {
      const c = funcArgsStr[i];
      if (c === '(') {
        parenDepth++;
        currentArg += c;
      } else if (c === ')') {
        parenDepth--;
        currentArg += c;
      } else if (c === ',' && parenDepth === 0) {
        funcArgs.push(currentArg.trim());
        currentArg = '';
      } else {
        currentArg += c;
      }
    }
    if (currentArg.trim().length > 0) {
      funcArgs.push(currentArg.trim());
    }

    return { funcName, funcArgs };
  }

  isFunctionCall(str: string) {
    const numOpenParens = (str.match(/\(/g) ?? []).length;
    const numCloseParens = (str.match(/\)/g) ?? []).length;
    if (numOpenParens > 0) {
      if (numCloseParens === numOpenParens) {
        return true;
      }
      throw new Error(`Invalid function call: ${str}`);
    } else if (numCloseParens > 0) {
      if (numOpenParens === numCloseParens) {
        return true;
      }
      throw new Error(`Invalid function call: ${str}`);
    }
    return false;
  }

  boolFunctions: Record<string, (...args: string[]) => boolean> = {
    IS: (a: string) => {
      if (a == 'true') {
        return true;
      }
      if (a == 'false') {
        return false;
      }
      const vStr = getStorage(this.storage, a) ?? '';
      if (vStr == '0' || vStr == 'false' || !vStr) {
        return false;
      }
      return true;
    },
    ISNOT: (a: string) => {
      return !this.boolFunctions.IS(a);
    },
    EQ: (a: string, b: string) => {
      const aStorage = getStorage(this.storage, a);
      const bStorage = getStorage(this.storage, b);

      if (aStorage !== undefined && bStorage !== undefined) {
        return aStorage === bStorage;
      } else if (aStorage !== undefined && bStorage === undefined) {
        const numB = parseFloat(b);
        if (!isNaN(numB)) {
          return aStorage === b;
        }
        return false;
      } else if (aStorage === undefined && bStorage !== undefined) {
        const numA = parseFloat(a);
        if (!isNaN(numA)) {
          return a === bStorage;
        }
        return false;
      }
      if (!isNaN(parseFloat(a)) && !isNaN(parseFloat(b))) {
        return a == b;
      }
      return false;
    },
    NEQ: (a: string, b: string) => {
      return getStorage(this.storage, a) !== getStorage(this.storage, b);
    },
    GT: (a: string, b: string) => {
      const v1 = parseFloat(getStorage(this.storage, a) ?? '0');
      const v2 = parseFloat(b);
      if (isNaN(v1) || isNaN(v2)) {
        return false;
      }
      return v1 > v2;
    },
    GTE: (a: string, b: string) => {
      const v1 = parseFloat(getStorage(this.storage, a) ?? '0');
      const v2 = parseFloat(b);
      if (isNaN(v1) || isNaN(v2)) {
        return false;
      }
      return v1 >= v2;
    },
    LT: (a: string, b: string) => {
      const v1 = parseFloat(getStorage(this.storage, a) ?? '0');
      const v2 = parseFloat(b);
      if (isNaN(v1) || isNaN(v2)) {
        return false;
      }
      return v1 < v2;
    },
    LTE: (a: string, b: string) => {
      const v1 = parseFloat(getStorage(this.storage, a) ?? '0');
      const v2 = parseFloat(b);
      if (isNaN(v1) || isNaN(v2)) {
        return false;
      }
      return v1 <= v2;
    },
    ALL: (...args: string[]) => {
      return args.every((arg) => {
        return arg === 'true' || Boolean(getStorage(this.storage, arg));
      });
    },
    ANY: (...args: string[]) => {
      return args.some((arg) => {
        return arg === 'true' || Boolean(getStorage(this.storage, arg));
      });
    },
    HAS_ITEM: (itemName: string) => {
      const v = getStorage(this.storage, 'vars.items.' + itemName);
      return Boolean(v && v !== '0' && v !== 'false');
    },
    ONCE: (a: string) => {
      const onceKey = 'once.' + a;
      const v = getStorage(this.storage, onceKey);
      if (v === 'true') {
        return false;
      }
      if (!this.onceKeysToCommit.includes(onceKey)) {
        this.onceKeysToCommit.push(onceKey);
      }
      return true;
    },
    QUEST_IS_STARTED: (questName: string) => {
      return questIsStarted(this.storage, questName);
    },
    QUEST_IS_COMPLETE: (questName: string) => {
      return questIsComplete(this.storage, questName);
    },
    QUEST_STEP_EQ: (questName: string, stepId: string) => {
      return questStepEq(this.storage, this.quests, questName, stepId);
    },
    QUEST_STEP_COMPLETED: (questName: string, stepId: string) => {
      return questStepIsCompleted(this.storage, questName, stepId);
    },
    QUEST_SUB_STEP_SHOWN: (
      questName: string,
      stepId: string,
      subStepId: string,
    ) => {
      return questSubStepIsShown(
        this.storage,
        this.quests,
        questName,
        stepId,
        subStepId,
      );
    },
  };

  evalFunc(funcName: keyof typeof this.boolFunctions, ...funcArgs: string[]) {
    if (funcName in this.boolFunctions) {
      const result = this.boolFunctions[funcName](...funcArgs);
      return result;
    } else {
      throw new Error(`Conditional function '${funcName}' not found.`);
    }
  }

  evalCondition(str: string) {
    const conditionStr = str.trim();
    if (conditionStr === '') {
      return true;
    }
    if (conditionStr === 'true') {
      return true;
    }
    if (conditionStr === 'false') {
      return false;
    }
    if (this.isFunctionCall(str)) {
      const { funcName, funcArgs } = this.parseFunctionCall(str);
      const newFuncArgs: string[] = [];
      for (const arg of funcArgs) {
        if (this.isFunctionCall(arg)) {
          const result = this.evalCondition(arg);
          newFuncArgs.push(result ? 'true' : 'false');
        } else {
          newFuncArgs.push(arg);
        }
      }
      return this.evalFunc(
        funcName as keyof typeof this.boolFunctions,
        ...newFuncArgs
      );
    } else {
      throw new Error(`Invalid condition: ${this.baseConditionStr}`);
    }
  }
}

class StringEvaluator {
  storage: Record<string, any>;
  baseStringStr: string;
  quests: QuestTemplate[];
  questUpdated = false;
  receivedItemNames: string[] = [];

  constructor(
    storage: Record<string, any>,
    baseStringStr: string,
    quests: QuestTemplate[] = [],
  ) {
    this.storage = storage;
    this.baseStringStr = baseStringStr;
    this.quests = quests;
  }

  stringFunctions: Record<string, (...args: string[]) => any> = {
    GET: (a: string) => {
      return getStorage(this.storage, a);
    },
    SET_BOOL: (a: string, b: string = 'true') => {
      const v = b !== 'false';
      setStorage(this.storage, a, String(v));
    },
    SET_NUM: (a: string, b: string) => {
      const n = parseFloat(b);
      if (isNaN(n)) {
        throw new Error(`Invalid number value: ${b}`);
      }
      setStorage(this.storage, a, String(n));
    },
    MOD_NUM: (a: string, b: string) => {
      const n = parseFloat(b);
      if (isNaN(n)) {
        throw new Error(`Invalid number value: ${b}`);
      }
      const current = getStorage(this.storage, a);
      const currentN = parseFloat(current || '0');
      if (isNaN(currentN)) {
        throw new Error(`Variable ${a} is not a number`);
      }
      setStorage(this.storage, a, String(currentN + n));
    },
    SET_STR: (a: string, b: string) => {
      setStorage(this.storage, a, b);
    },
    SETUP_DISPOSITION: (_characterName: string) => {
      // noop
    },
    START_QUEST: (questName: string) => {
      startQuest(this.storage, this.quests, questName);
      this.questUpdated = true;
    },
    SET_QUEST_STEP_EQ: (questName: string, stepId: string) => {
      setQuestStepEq(this.storage, questName, stepId);
      this.questUpdated = true;
    },
    COMPLETE_QUEST_STEP: (questName: string, stepId: string) => {
      completeQuestStep(this.storage, questName, stepId);
      this.questUpdated = true;
    },
    SHOW_QUEST_SUB_STEP: (
      questName: string,
      stepId: string,
      subStepId: string,
    ) => {
      showQuestSubStep(this.storage, this.quests, questName, stepId, subStepId);
      this.questUpdated = true;
    },
    HIDE_QUEST_SUB_STEP: (
      questName: string,
      stepId: string,
      subStepId: string,
    ) => {
      hideQuestSubStep(this.storage, this.quests, questName, stepId, subStepId);
      this.questUpdated = true;
    },
    COMPLETE_QUEST_SUB_STEP: (
      questName: string,
      stepId: string,
      subStepId: string,
    ) => {
      completeQuestSubStep(
        this.storage,
        this.quests,
        questName,
        stepId,
        subStepId,
      );
      this.questUpdated = true;
    },
    COMPLETE_QUEST: (questName: string) => {
      completeQuest(this.storage, questName);
      this.questUpdated = true;
    },
    SPAWN_CH: (_chName: string) => {
      // noop
    },
    DESPAWN_CH: (_chName: string) => {
      // noop
    },
    CHANGE_TILE_AT: (_x: string, _y: string, _tileName: string) => {
      // noop
    },
    TELEPORT_TO: (_x: string, _y: string, _mapName: string) => {
      // noop
    },
    ADD_ITEM_AT: (_x: string, _y: string, _itemName: string) => {
      // noop
    },
    REMOVE_ITEM_AT: (_x: string, _y: string, _itemName: string) => {
      // noop
    },
    ADD_ITEM_TO_PLAYER: (itemName: string) => {
      const key = 'vars.items.' + itemName;
      this.stringFunctions.MOD_NUM(key, '1');
      this.receivedItemNames.push(itemName);
    },
    REMOVE_ITEM_FROM_PLAYER: (itemName: string) => {
      const key = 'vars.items.' + itemName;
      this.stringFunctions.MOD_NUM(key, '-1');
    },
    OPEN_SHOP: (_shopName: string) => {
      // noop
    },
    SET_PORT: (characterName: string = '') => {
      if (!characterName) {
        setStorage(this.storage, 'tmp.talk.port', '');
        return;
      }
      setStorage(this.storage, 'tmp.talk.port', characterName);
    },
  };

  parseFunctionCall(str: string) {
    const funcName = str.split('(')[0];
    let funcArgsStr = str.split('(')[1];
    funcArgsStr = funcArgsStr.slice(0, funcArgsStr.lastIndexOf(')'));
    const funcArgs = funcArgsStr.split(',').map((arg) => arg.trim());
    return { funcName, funcArgs };
  }

  isFunctionCall(str: string) {
    return str.includes('(') && str.includes(')');
  }

  evalStr(str: string) {
    // console.log('evalStr', str);
    if (this.isFunctionCall(str)) {
      const { funcName, funcArgs } = this.parseFunctionCall(str);
      // console.log('- args', funcName, funcArgs);
      if (funcName in this.stringFunctions) {
        return this.stringFunctions[
          funcName as keyof typeof this.stringFunctions
        ](...funcArgs);
      } else {
        throw new Error(
          `Function '${funcName}' not found: ${this.baseStringStr}`
        );
      }
    } else {
      throw new Error(`Invalid eval string: ${this.baseStringStr}`);
    }
  }
}

export type EventRunnerLogEntry = {
  type: 'text' | 'choice' | 'continue' | 'storage' | 'journal' | 'item';
  text: string;
  nodeId?: string;
  choiceKey?: string;
};

export class EventRunner {
  storage: Record<string, any>;
  gameEvent: GameEvent;
  gameEvents: GameEvent[];
  quests: QuestTemplate[];
  items: ItemTemplate[];
  currentNodeId: string;

  displayText: string = '';
  logEntries: EventRunnerLogEntry[] = [];
  displayTextChoices: {
    execStr: string;
    text: string;
    prefix: string;
    next: string;
    onceKeysToCommit: string[];
    choiceKey: string;
  }[] = [];

  errors: {
    nodeId: string;
    message: string;
  }[] = [];
  pendingJournalNotice = false;
  pendingReceivedItemNames: string[] = [];

  constructor(
    initialStorage: Record<string, any> = {},
    gameEvent: GameEvent,
    gameEvents: GameEvent[],
    quests: QuestTemplate[] = [],
    items: ItemTemplate[] = [],
  ) {
    this.storage = initialStorage;
    this.gameEvent = gameEvent;
    this.gameEvents = gameEvents;
    this.quests = quests;
    this.items = items;
    this.currentNodeId = gameEvent.children.some((node) => node.id === 'root')
      ? 'root'
      : gameEvent.children[0].id;
  }

  getCurrentNode() {
    return this.gameEvent.children.find(
      (node) => node.id === this.currentNodeId
    );
  }

  replaceVariables(text: string, highlight: boolean = false) {
    // console.log('replaceVariables', text);
    const vars = getVarsFromNode(this.gameEvent, this.gameEvents);
    text = text.trim();

    for (const variable of vars) {
      // const value = getStorage(this.storage, variable.value);
      const value = variable.value;
      text = text.replaceAll(`@${variable.key}`, String(value));
      if (highlight) {
        text = text.replaceAll(
          `@${variable.key}`,
          `<span style="color: yellow;">${String(value)}</span>`
        );
      } else {
        text = text.replaceAll(`@${variable.key}`, String(value));
      }
    }

    return text;
  }

  evalExecStr(str: string) {
    str = str.trim();
    console.log(`evalExecStr: "${str}"`);
    if (str === '') {
      return true;
    }
    if (str === 'true') {
      return true;
    }
    if (str === 'false') {
      return false;
    }

    const stringEvaluator = new StringEvaluator(this.storage, str, this.quests);
    try {
      const result = stringEvaluator.evalStr(str);
      if (stringEvaluator.questUpdated) {
        this.pendingJournalNotice = true;
      }
      for (const itemName of stringEvaluator.receivedItemNames) {
        if (itemName && !this.pendingReceivedItemNames.includes(itemName)) {
          this.pendingReceivedItemNames.push(itemName);
        }
      }
      return result;
    } catch (error: unknown) {
      this.errors.push({
        nodeId: this.currentNodeId,
        message: (error as Error).message,
      });
      console.log(
        `ERROR evaluating string: '${str}'`,
        this.currentNodeId,
        error
      );
      return false;
    }
  }

  evalCondition(conditionStr: string) {
    // console.log('eval condition', conditionStr);
    const conditionEvaluator = new ConditionEvaluator(
      this.storage,
      conditionStr,
      this.quests,
    );
    try {
      const result = Boolean(conditionEvaluator.evalCondition(conditionStr));
      console.log('evalCondition with evaluator', { conditionStr, result });
      return {
        result,
        onceKeysToCommit: conditionEvaluator.onceKeysToCommit,
      };
    } catch (error: unknown) {
      this.errors.push({
        nodeId: this.currentNodeId,
        message: (error as Error).message,
      });
      console.log(
        `ERROR evaluating condition: '${conditionStr}'`,
        this.currentNodeId,
        error
      );
      return {
        result: false,
        onceKeysToCommit: [],
      };
    }
  }

  static getConditionFunctions() {
    return Object.keys(ConditionEvaluator.prototype.boolFunctions);
  }

  static getExecFunctions() {
    return Object.keys(StringEvaluator.prototype.stringFunctions);
  }

  commitOnceKeys(onceKeysToCommit: string[]) {
    for (const onceKeyToCommit of onceKeysToCommit) {
      setStorage(this.storage, onceKeyToCommit, 'true');
    }
  }

  appendToLog(text: string, nodeId: string = this.currentNodeId) {
    const plain = text.trim();
    if (!plain) {
      return;
    }
    this.logEntries.push({ type: 'text', text: plain, nodeId });
  }

  flushJournalNotice() {
    if (!this.pendingJournalNotice) {
      return;
    }
    this.pendingJournalNotice = false;
    // One grey line per player-facing stop, even if several quest calls
    // ran on this node or earlier in the auto-advance chain.
    for (let i = this.logEntries.length - 1; i >= 0; i--) {
      const type = this.logEntries[i].type;
      if (type === 'journal') {
        return;
      }
      if (type === 'choice' || type === 'continue') {
        break;
      }
    }
    this.logEntries.push({
      type: 'journal',
      text: JOURNAL_UPDATED_MESSAGE,
      nodeId: this.currentNodeId,
    });
  }

  itemReceivedLabel(itemName: string) {
    const item = this.items.find((entry) => entry.name === itemName);
    const label = item?.label?.trim();
    return label || itemName;
  }

  flushItemNotices() {
    const names = this.pendingReceivedItemNames;
    this.pendingReceivedItemNames = [];
    for (const itemName of names) {
      const text = itemReceivedNoticeText(this.itemReceivedLabel(itemName));
      let already = false;
      for (let i = this.logEntries.length - 1; i >= 0; i--) {
        const entry = this.logEntries[i];
        if (entry.type === 'item' && entry.text === text) {
          already = true;
          break;
        }
        if (entry.type === 'choice' || entry.type === 'continue') {
          break;
        }
      }
      if (already) {
        continue;
      }
      this.logEntries.push({
        type: 'item',
        text,
        nodeId: this.currentNodeId,
      });
    }
  }

  flushSystemNotices() {
    this.flushItemNotices();
    this.flushJournalNotice();
  }

  appendChoiceToLog(
    text: string,
    choiceKey: string,
    nodeId: string = this.currentNodeId
  ) {
    const plain = text.replace(/<[^>]*>/g, '').trim();
    if (!plain) {
      return;
    }
    this.logEntries.push({ type: 'choice', text: plain, choiceKey, nodeId });
  }

  recordChoiceSelection(index: number) {
    const choice = this.displayTextChoices[index];
    if (!choice) {
      return;
    }
    const label =
      (choice.prefix ? choice.prefix + ' ' : '') + choice.text;
    this.appendChoiceToLog(label, choice.choiceKey);
  }

  recordContinue() {
    this.logEntries.push({
      type: 'continue',
      text: '',
      nodeId: this.currentNodeId,
    });
  }

  appendEndStorageToLog() {
    if (this.logEntries.some((entry) => entry.type === 'storage')) {
      return;
    }
    this.appendToLog('End.', this.currentNodeId);
    this.logEntries.push({
      type: 'storage',
      text: JSON.stringify(this.storage, null, 2),
      nodeId: this.currentNodeId,
    });
  }

  resolveChoiceText(choice: Choice): {
    text: string;
    onceKeysToCommit: string[];
  } {
    if (choice.switchText?.length) {
      for (const switchText of choice.switchText) {
        if (switchText.conditionStr) {
          const obj = this.evalCondition(
            this.replaceVariables(switchText.conditionStr)
          );
          if (obj.result) {
            return {
              text: this.replaceVariables(switchText.text, false),
              onceKeysToCommit: obj.onceKeysToCommit,
            };
          }
        }
      }
    }
    return {
      text: this.replaceVariables(choice.text, false),
      onceKeysToCommit: [],
    };
  }

  getPauseState():
    | 'continue'
    | 'choice'
    | 'end'
    | 'error'
    | 'done' {
    if (this.errors.length > 0) {
      return 'error';
    }
    const node = this.getCurrentNode();
    if (!node) {
      return 'done';
    }
    if (node.eventChildType === GameEventChildType.END) {
      return 'end';
    }
    if (node.eventChildType === GameEventChildType.CHOICE) {
      return 'choice';
    }
    if (node.eventChildType === GameEventChildType.EXEC) {
      const execNode = node as GameEventChildExec;
      const text = this.replaceVariables(execNode.p, false);
      if (text && !execNode.autoAdvance) {
        return 'continue';
      }
    }
    return 'done';
  }

  advance(
    nextNodeId: string,
    {
      onceKeysToCommit,
      execStr,
    }: { onceKeysToCommit: string[]; execStr: string }
  ) {
    if (this.errors.length > 0) {
      return;
    }

    console.log('ADVANCE', nextNodeId, { onceKeysToCommit, execStr });

    this.commitOnceKeys(onceKeysToCommit);
    if (execStr) {
      for (const strLine of splitExecStatements(execStr)) {
        this.evalExecStr(this.replaceVariables(strLine));
      }
    }

    this.displayText = '';
    this.displayTextChoices = [];

    // const prevNodeId = this.currentNodeId;
    this.currentNodeId = nextNodeId;
    const currentNode = this.getCurrentNode();
    if (nextNodeId === '' || !currentNode) {
      return;
    }

    if (currentNode?.eventChildType === GameEventChildType.EXEC) {
      const execNode = currentNode as GameEventChildExec;
      for (const strLine of splitExecStatements(execNode.execStr)) {
        this.evalExecStr(this.replaceVariables(strLine));
      }
      const text = this.replaceVariables(execNode.p, false);
      this.displayText = text;
      if (text) {
        this.appendToLog(text);
      }
      if (!text || execNode.autoAdvance) {
        this.advance(execNode.next, { onceKeysToCommit: [], execStr: '' });
      }
      this.flushSystemNotices();
    } else if (currentNode?.eventChildType === GameEventChildType.CHOICE) {
      const choiceNode = currentNode as GameEventChildChoice;
      const choiceText = this.replaceVariables(choiceNode.text, false);
      this.displayText = choiceText;
      if (choiceText) {
        this.appendToLog(choiceText);
      }
      this.flushSystemNotices();
      this.displayTextChoices = choiceNode.choices
        .map((choice, choiceIndex) => {
          const obj = choice.conditionStr
            ? this.evalCondition(
                this.replaceVariables(choice.conditionStr ?? '')
              )
            : { result: true, onceKeysToCommit: [] };
          const resolved = this.resolveChoiceText(choice);
          return {
            prefix: this.replaceVariables(choice.prefixText ?? '', false),
            text: this.replaceVariables(resolved.text, true),
            execStr: this.replaceVariables(choice.evalStr ?? '', false),
            next: choice.next,
            onceKeysToCommit: [
              ...obj.onceKeysToCommit,
              ...resolved.onceKeysToCommit,
            ],
            choiceKey: `${this.currentNodeId}:${choiceIndex}`,
            result: obj.result,
          };
        })
        .filter((choice) => choice.result);
    } else if (currentNode?.eventChildType === GameEventChildType.SWITCH) {
      const switchNode = currentNode as GameEventChildSwitch;
      let found = false;
      for (let i = 0; i < switchNode.cases.length; i++) {
        const c = switchNode.cases[i];
        // console.log('switch case', i, c, switchNode);
        const obj = this.evalCondition(
          this.replaceVariables(c.conditionStr ?? '')
        );
        if (obj.result) {
          this.advance(c.next, {
            onceKeysToCommit: obj.onceKeysToCommit,
            execStr: '',
          });
          found = true;
          break;
        }
      }
      if (!found) {
        this.advance(switchNode.defaultNext, {
          onceKeysToCommit: [],
          execStr: '',
        });
      }
    } else if (currentNode?.eventChildType === GameEventChildType.END) {
      this.flushSystemNotices();
      this.appendEndStorageToLog();
    }
  }
}

// editor code only, not used in c++
export function getAvailableFuncs(): string[] {
  const stringEvaluator = new StringEvaluator({}, '');
  const conditionEvaluator = new ConditionEvaluator({}, '');

  // Helper function to extract parameter names from function source
  const extractParams = (func: {
    toString(): string;
    length: number;
  }): string[] => {
    const funcStr = func.toString();
    // Match arrow function parameters: (a, b) => or (a: string, b: string) => or (...args) =>
    const arrowMatch = funcStr.match(/^\(([^)]*)\)\s*=>/);
    if (arrowMatch) {
      const paramsStr = arrowMatch[1].trim();
      if (paramsStr === '') return [];
      // Handle rest parameters
      if (paramsStr.startsWith('...')) {
        // Extract just the parameter name from "...args: string[]"
        const restMatch = paramsStr.match(/^\.\.\.(\w+)/);
        return restMatch ? [`...${restMatch[1]}`] : [paramsStr];
      }
      // Split by comma and extract parameter names (strip type annotations)
      return paramsStr.split(',').map((p) => {
        const trimmed = p.trim();
        // Extract parameter name, handling type annotations like "a: string"
        const nameMatch = trimmed.match(/^_?(\w+)/);
        return nameMatch ? nameMatch[1] : trimmed;
      });
    }
    // Fallback: use function.length to generate generic names
    const paramCount = func.length;
    if (paramCount === 0) return [];
    return Array(paramCount)
      .fill(0)
      .map((_, i) => `arg${i + 1}`);
  };

  const result: Array<{
    name: string;
    args: string[];
    argCount: number;
    type: 'string' | 'bool';
  }> = [];

  // Add string functions
  for (const funcName of Object.keys(stringEvaluator.stringFunctions)) {
    const func = stringEvaluator.stringFunctions[funcName];
    const args = extractParams(func);
    const isVariadic = args.some((arg) => arg.startsWith('...'));
    const argCount = isVariadic ? -1 : args.length; // -1 indicates variadic
    result.push({
      name: funcName,
      args,
      argCount,
      type: 'string',
    });
  }

  // Add bool functions
  for (const funcName of Object.keys(conditionEvaluator.boolFunctions)) {
    const func = conditionEvaluator.boolFunctions[funcName];
    const args = extractParams(func);
    const isVariadic = args.some((arg) => arg.startsWith('...'));
    const argCount = isVariadic ? -1 : args.length; // -1 indicates variadic
    result.push({
      name: funcName,
      args,
      argCount,
      type: 'bool',
    });
  }

  return result
    .map((func) => `${func.name}(${func.args.join(', ')}): ${func.type}`)
    .sort();
}

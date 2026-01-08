import {
  GameEvent,
  GameEventChildChoice,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildType,
} from '../../types/assets';
import { getVarsFromNode } from '../nodeHelpers';

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

class ConditionEvaluator {
  storage: Record<string, any>;
  baseConditionStr: string;
  onceKeysToCommit: string[] = [];

  constructor(storage: Record<string, any>, baseConditionStr: string) {
    this.storage = storage;
    this.baseConditionStr = baseConditionStr;
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
    FUNC: (funcName: string, ...args: string[]) => {
      if (funcName === 'HasItem') {
        return Boolean(getStorage(this.storage, 'vars.items.' + args[0]));
      }
      if (funcName === 'QuestStarted') {
        return Boolean(
          getStorage(this.storage, 'vars.quests.' + args[0] + '.started')
        );
      }
      if (funcName === 'QuestCompleted') {
        return Boolean(
          getStorage(this.storage, 'vars.quests.' + args[0] + '.completed')
        );
      }
      if (funcName === 'QuestStepEq') {
        return Boolean(
          getStorage(this.storage, 'vars.quests.' + args[0] + '.step') ===
            args[1]
        );
      }
      console.error('Invalid FUNC conditional: ', { funcName, args });
      return false;
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
    let conditionStr = str.trim();
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
      let { funcName, funcArgs } = this.parseFunctionCall(str);
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

  constructor(storage: Record<string, any>, baseStringStr: string) {
    this.storage = storage;
    this.baseStringStr = baseStringStr;
  }

  stringFunctions: Record<string, (...args: string[]) => any> = {
    GET: (a: string) => {
      return getStorage(this.storage, a);
    },
    SET_BOOL: (a: string, b: string = 'true') => {
      let v = true;
      if (b === 'true') {
        v = true;
      } else if (b === 'false') {
        v = false;
      } else {
        v = true;
      }
      // console.log(`SET_BOOL: "${a}" = "${v}"`, b);
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
    SETUP_DISPOSITION: (characterName: string) => {
      // noop
    },
    START_QUEST: (questName: string) => {
      // noop
      setStorage(this.storage, 'vars.quests.' + questName + '.step', '1');
      setStorage(this.storage, 'vars.quests.' + questName + '.started', 'true');
    },
    ADVANCE_QUEST: (questName: string, stepId: string) => {
      // noop
      setStorage(this.storage, 'vars.quests.' + questName + '.step', stepId);
    },
    COMPLETE_QUEST: (questName: string) => {
      // noop
      setStorage(
        this.storage,
        'vars.quests.' + questName + '.completed',
        'true'
      );
    },
    SPAWN_CH: (chName: string) => {
      // noop
    },
    DESPAWN_CH: (chName: string) => {
      // noop
    },
    CHANGE_TILE_AT: (x: string, y: string, tileName: string) => {
      // noop
    },
    TELEPORT_TO: (x: string, y: string, mapName: string) => {
      // noop
    },
    ADD_ITEM_AT: (x: string, y: string, itemName: string) => {
      // noop
    },
    REMOVE_ITEM_AT: (x: string, y: string, itemName: string) => {
      // noop
    },
    ADD_ITEM_TO_PLAYER: (itemName: string) => {
      const key = 'vars.items.' + itemName;
      this.stringFunctions.MOD_NUM(key, '1');
    },
    REMOVE_ITEM_FROM_PLAYER: (itemName: string) => {
      const key = 'vars.items.' + itemName;
      this.stringFunctions.MOD_NUM(key, '-1');
    },
    OPEN_SHOP: (shopName: string) => {
      // noop
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
      let { funcName, funcArgs } = this.parseFunctionCall(str);
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

export class EventRunner {
  storage: Record<string, any>;
  gameEvent: GameEvent;
  gameEvents: GameEvent[];
  currentNodeId: string;

  displayText: string = '';
  displayTextChoices: {
    execStr: string;
    text: string;
    prefix: string;
    next: string;
    onceKeysToCommit: string[];
  }[] = [];

  errors: {
    nodeId: string;
    message: string;
  }[] = [];

  constructor(
    initialStorage: Record<string, any> = {},
    gameEvent: GameEvent,
    gameEvents: GameEvent[]
  ) {
    this.storage = initialStorage;
    this.gameEvent = gameEvent;
    this.gameEvents = gameEvents;
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

    const stringEvaluator = new StringEvaluator(this.storage, str);
    try {
      const result = stringEvaluator.evalStr(str);
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
      conditionStr
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
      const strLines = execStr.split('\n');
      for (const strLine of strLines) {
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
      const strLines = execNode.execStr.split('\n');
      for (const strLine of strLines) {
        this.evalExecStr(this.replaceVariables(strLine));
      }
      this.displayText = this.replaceVariables(execNode.p, true);
      if (!this.displayText) {
        this.advance(execNode.next, { onceKeysToCommit: [], execStr: '' });
      }
    } else if (currentNode?.eventChildType === GameEventChildType.CHOICE) {
      const choiceNode = currentNode as GameEventChildChoice;
      this.displayText = this.replaceVariables(choiceNode.text, true);
      this.displayTextChoices = choiceNode.choices
        .map((choice) => {
          const obj = choice.conditionStr
            ? this.evalCondition(
                this.replaceVariables(choice.conditionStr ?? '')
              )
            : { result: true, onceKeysToCommit: [] };
          return {
            prefix: this.replaceVariables(choice.prefixText ?? '', false),
            text: this.replaceVariables(choice.text, true),
            execStr: this.replaceVariables(choice.evalStr ?? '', false),
            next: choice.next,
            onceKeysToCommit: obj.onceKeysToCommit,
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
    }
  }
}

// editor code only, not used in c++
export function getAvailableFuncs(): string[] {
  const stringEvaluator = new StringEvaluator({}, '');
  const conditionEvaluator = new ConditionEvaluator({}, '');

  // Helper function to extract parameter names from function source
  const extractParams = (func: Function): string[] => {
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
        const nameMatch = trimmed.match(/^(\w+)/);
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

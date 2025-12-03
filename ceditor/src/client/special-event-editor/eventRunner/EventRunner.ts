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

  constructor(storage: Record<string, any>, baseConditionStr: string) {
    this.storage = storage;
    this.baseConditionStr = baseConditionStr;
  }

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

  boolFunctions: Record<string, (...args: string[]) => boolean> = {
    IS: (a: string) => {
      const v = getStorage(this.storage, a);
      if (v === 'false') {
        return false;
      }
      return Boolean(v);
    },
    ISNOT: (a: string) => {
      return !Boolean(getStorage(this.storage, a));
    },
    EQ: (a: string, b: string) => {
      return getStorage(this.storage, a) === getStorage(this.storage, b);
    },
    NEQ: (a: string, b: string) => {
      return getStorage(this.storage, a) !== getStorage(this.storage, b);
    },
    GT: (a: string, b: string) => {
      return (
        (getStorage(this.storage, a) ?? 0) > (getStorage(this.storage, b) ?? 0)
      );
    },
    GTE: (a: string, b: string) => {
      return (
        (getStorage(this.storage, a) ?? 0) >= (getStorage(this.storage, b) ?? 0)
      );
    },
    LT: (a: string, b: string) => {
      return (
        (getStorage(this.storage, a) ?? 0) < (getStorage(this.storage, b) ?? 0)
      );
    },
    LTE: (a: string, b: string) => {
      return (
        (getStorage(this.storage, a) ?? 0) <= (getStorage(this.storage, b) ?? 0)
      );
    },
    ALL: (...args: string[]) => {
      return args.every((arg) => Boolean(getStorage(this.storage, arg)));
    },
    ANY: (...args: string[]) => {
      return args.some((arg) => Boolean(getStorage(this.storage, arg)));
    },
  };

  evalArg(arg: string) {
    if (this.isFunctionCall(arg)) {
      let { funcName, funcArgs } = this.parseFunctionCall(arg);
      if (funcName in this.boolFunctions) {
        return this.boolFunctions[funcName as keyof typeof this.boolFunctions](
          ...funcArgs
        );
      } else {
        throw new Error(`Function ${funcName} not found`);
      }
    } else {
      return arg;
    }
  }

  evalCondition(str: string) {
    console.log('evalCondition', str);
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
      if (funcName in this.boolFunctions) {
        const args: any[] = [];
        for (const arg of funcArgs) {
          const evalArg = this.evalArg(arg);
          args.push(evalArg);
        }
        return this.boolFunctions[funcName as keyof typeof this.boolFunctions](
          ...args
        );
      } else {
        throw new Error(
          `Function' ${funcName}' not found in condition: ${this.baseConditionStr}`
        );
      }
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
    SET_BOOL: (a: string, b: string) => {
      let v = true;
      if (b === 'true') {
        v = true;
      } else if (b === 'false') {
        v = false;
      } else {
        throw new Error(`Invalid boolean value: ${b}`);
      }
      setStorage(this.storage, a, String(v));
    },
    SET_NUM: (a: string, b: string) => {
      const n = parseFloat(b);
      if (isNaN(n)) {
        throw new Error(`Invalid number value: ${b}`);
      }
      setStorage(this.storage, a, String(n));
    },
    SET_STR: (a: string, b: string) => {
      setStorage(this.storage, a, b);
    },
    MOD: (a: string, b: string) => {
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
    START_QUEST: (questName: string) => {
      // noop
    },
    ADVANCE_QUEST: (questName: string, stepId: string) => {
      // noop
    },
    COMPLETE_QUEST: (questName: string) => {
      // noop
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
    console.log('evalStr', str);
    if (this.isFunctionCall(str)) {
      let { funcName, funcArgs } = this.parseFunctionCall(str);
      console.log('- args', funcName, funcArgs);
      if (funcName in this.stringFunctions) {
        return this.stringFunctions[
          funcName as keyof typeof this.stringFunctions
        ](...funcArgs);
      } else {
        throw new Error(
          `Function '${funcName}' not found in string: ${this.baseStringStr}`
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
  displayTextChoices: { text: string; next: string }[] = [];

  errors: string[] = [];

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
    const vars = getVarsFromNode(this.gameEvent, this.gameEvents);
    text = text.trim();

    const evalMatches1 = text.match(/@([\w\d_]+)/g);
    for (const match of evalMatches1 || []) {
      const foundVar = vars.find((v) => v.key === match.slice(1));
      if (foundVar) {
        const value = getStorage(this.storage, foundVar.value);
        if (highlight) {
          text = text.replaceAll(
            `${match}`,
            `<span style="color: yellow;">${String(value)}</span>`
          );
        } else {
          text = text.replaceAll(`${match}`, String(value));
        }
      } else {
        this.errors.push(`Variable ${match} not found`);
      }
    }

    return text;
  }

  evalExecStr(str: string) {
    str = str.trim();
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
      this.errors.push((error as Error).message);
      console.error(`error evaluating string: '${str}'`, error);
      return false;
    }
  }

  evalCondition(conditionStr: string) {
    const conditionEvaluator = new ConditionEvaluator(
      this.storage,
      conditionStr
    );
    try {
      const result = conditionEvaluator.evalCondition(conditionStr);
      console.log('evalCondition', { conditionStr, result });
      return result;
    } catch (error: unknown) {
      this.errors.push((error as Error).message);
      console.error(`error evaluating condition: '${conditionStr}'`, error);
      return false;
    }
  }

  static getConditionFunctions() {
    return Object.keys(ConditionEvaluator.prototype.boolFunctions);
  }

  static getExecFunctions() {
    return Object.keys(StringEvaluator.prototype.stringFunctions);
  }

  advance(nextNodeId: string) {
    if (this.errors.length > 0) {
      return;
    }

    this.displayText = '';
    this.displayTextChoices = [];

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
      console.log('storage after exec:', this.storage);
      if (!this.displayText) {
        this.advance(execNode.next);
      }
    } else if (currentNode?.eventChildType === GameEventChildType.CHOICE) {
      const choiceNode = currentNode as GameEventChildChoice;
      this.displayText = this.replaceVariables(choiceNode.text, true);
      this.displayTextChoices = choiceNode.choices
        .filter((choice) => {
          return choice.conditionStr
            ? this.evalCondition(this.replaceVariables(choice.conditionStr))
            : true;
        })
        .map((choice) => ({
          text: this.replaceVariables(choice.text, true),
          next: choice.next,
        }));
    } else if (currentNode?.eventChildType === GameEventChildType.SWITCH) {
      const switchNode = currentNode as GameEventChildSwitch;
      let found = false;
      for (const c of switchNode.cases) {
        if (this.evalCondition(this.replaceVariables(c.conditionStr))) {
          this.advance(c.next);
          found = true;
          break;
        }
      }
      if (!found) {
        this.advance(switchNode.defaultNext);
      }
    }
  }
}

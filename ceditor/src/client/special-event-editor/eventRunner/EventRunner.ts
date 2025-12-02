import {
  GameEvent,
  GameEventChildChoice,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildType,
} from '../../types/assets';
import { getVarsFromNode } from '../nodeHelpers';

const setStorage = (storage: Record<string, any>, key: string, value: any) => {
  const keys = key.split('.');
  let curr = storage;
  for (let i = 0; i < keys.length - 1; i++) {
    if (typeof curr[keys[i]] !== 'object' || curr[keys[i]] === null) {
      curr[keys[i]] = {};
    }
    curr = curr[keys[i]];
  }
  curr[keys[keys.length - 1]] = String(value);
};

const getStorage = (storage: Record<string, any>, key: string): any => {
  const keys = key.split('.');
  let curr = storage;
  for (let i = 0; i < keys.length; i++) {
    curr = curr[keys[i]];
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
      return Boolean(getStorage(this.storage, a));
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
      return getStorage(this.storage, a) > getStorage(this.storage, b);
    },
    GTE: (a: string, b: string) => {
      return getStorage(this.storage, a) >= getStorage(this.storage, b);
    },
    LT: (a: string, b: string) => {
      return getStorage(this.storage, a) < getStorage(this.storage, b);
    },
    LTE: (a: string, b: string) => {
      return getStorage(this.storage, a) <= getStorage(this.storage, b);
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
      setStorage(this.storage, a, v);
    },
    SET_NUM: (a: string, b: string) => {
      const n = parseFloat(b);
      if (isNaN(n)) {
        throw new Error(`Invalid number value: ${b}`);
      }
      setStorage(this.storage, a, n);
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
      const currentN = parseFloat(current);
      if (isNaN(currentN)) {
        throw new Error(`Variable ${a} is not a number`);
      }
      setStorage(this.storage, a, currentN + n);
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

  replaceVariables(text: string) {
    text = text.trim();
    const vars = getVarsFromNode(this.gameEvent, this.gameEvents);

    // the slow way
    for (const v of vars) {
      text = text.replaceAll(`@${v.key}`, v.value.toString());
    }
    return text;
  }

  evalStr(str: string) {
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
      return result;
    } catch (error: unknown) {
      this.errors.push((error as Error).message);
      console.error(`error evaluating condition: '${conditionStr}'`, error);
      return false;
    }
  }

  advance(nextNodeId: string) {
    if (this.errors.length > 0) {
      return;
    }

    this.currentNodeId = nextNodeId;
    const currentNode = this.getCurrentNode();
    if (nextNodeId === '' || !currentNode) {
      return;
    }
    if (currentNode?.eventChildType === GameEventChildType.EXEC) {
      const execNode = currentNode as GameEventChildExec;
      const strLines = execNode.execStr.split('\n');
      for (const strLine of strLines) {
        this.evalStr(this.replaceVariables(strLine));
      }
      this.displayText = this.replaceVariables(execNode.p);
      console.log('evald', this.storage);
      if (!this.displayText) {
        this.advance(execNode.next);
      }
    } else if (currentNode?.eventChildType === GameEventChildType.CHOICE) {
      const choiceNode = currentNode as GameEventChildChoice;
      this.displayTextChoices = choiceNode.choices
        .filter((choice) => {
          return choice.conditionStr
            ? this.evalCondition(this.replaceVariables(choice.conditionStr))
            : true;
        })
        .map((choice) => ({ text: choice.text, next: choice.next }));
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

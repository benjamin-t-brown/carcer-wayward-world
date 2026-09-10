import { analyzeEventGraph } from './eventGraph.js';
import { parseSpecialEventRecord } from './eventParser.js';
import type {
  EventChoice,
  EventChoiceNode,
  EventExecNode,
  EventNode,
  EventSwitchNode,
  SpecialEventRecord,
} from './types.js';

export type EventVariableIssueCode =
  'missing-import' | 'import-cycle' | 'duplicate-variable-key';

export interface EventVariableIssue {
  readonly code: EventVariableIssueCode;
  readonly message: string;
  readonly eventId: string;
  readonly importFrom?: string;
  readonly key?: string;
}

export interface ResolvedEventVariable {
  readonly eventId: string;
  readonly key: string;
  readonly value: string;
}

export interface EventVariablePlan {
  /** Variables in the same deterministic order used by the game runtime. */
  readonly variables: readonly ResolvedEventVariable[];
  /** First definition wins, matching sequential placeholder replacement. */
  readonly values: ReadonlyMap<string, string>;
  readonly visitedEventIds: readonly string[];
  readonly issues: readonly EventVariableIssue[];
}

function eventCatalog(
  event: SpecialEventRecord,
  importedEvents: readonly SpecialEventRecord[],
): ReadonlyMap<string, SpecialEventRecord> {
  const catalog = new Map<string, SpecialEventRecord>();
  for (const candidate of importedEvents) {
    if (!catalog.has(candidate.id)) catalog.set(candidate.id, candidate);
  }
  catalog.set(event.id, event);
  return catalog;
}

/**
 * Plan imports without evaluating expressions or changing persisted data.
 * An import marker imports all local variables from its target event; cycles are
 * stopped after the first visit, as they are in the C++ runtime.
 */
export function resolveEventVariables(
  event: SpecialEventRecord,
  importedEvents: readonly SpecialEventRecord[] = [],
): EventVariablePlan {
  const catalog = eventCatalog(event, importedEvents);
  const variables: ResolvedEventVariable[] = [];
  const values = new Map<string, string>();
  const visited = new Set<string>();
  const active = new Set<string>();
  const visitedEventIds: string[] = [];
  const issues: EventVariableIssue[] = [];

  const visit = (current: SpecialEventRecord): void => {
    if (active.has(current.id)) {
      issues.push({
        code: 'import-cycle',
        eventId: current.id,
        message: `Variable import cycle reaches event "${current.id}".`,
      });
      return;
    }
    if (visited.has(current.id)) return;

    visited.add(current.id);
    active.add(current.id);
    visitedEventIds.push(current.id);

    for (const variable of current.vars ?? []) {
      if ((variable.importFrom ?? '') !== '') continue;
      const key = variable.key ?? '';
      const value = variable.value ?? '';
      if (values.has(key)) {
        issues.push({
          code: 'duplicate-variable-key',
          eventId: current.id,
          key,
          message: `Variable "${key}" is already defined; the first value is used.`,
        });
      } else {
        values.set(key, value);
      }
      variables.push(Object.freeze({ eventId: current.id, key, value }));
    }

    for (const variable of current.vars ?? []) {
      const importFrom = variable.importFrom ?? '';
      if (importFrom === '') continue;
      const imported = catalog.get(importFrom);
      if (imported === undefined) {
        issues.push({
          code: 'missing-import',
          eventId: current.id,
          importFrom,
          message: `Event "${current.id}" imports missing event "${importFrom}".`,
        });
      } else if (active.has(importFrom)) {
        issues.push({
          code: 'import-cycle',
          eventId: current.id,
          importFrom,
          message: `Variable import from "${current.id}" to "${importFrom}" forms a cycle.`,
        });
      } else {
        visit(imported);
      }
    }
    active.delete(current.id);
  };

  visit(event);
  return Object.freeze({
    variables: Object.freeze(variables),
    values,
    visitedEventIds: Object.freeze(visitedEventIds),
    issues: Object.freeze(issues),
  });
}

export function replaceEventVariables(
  text: string,
  plan: Pick<EventVariablePlan, 'variables'>,
): string {
  let result = text.trim();
  for (const variable of plan.variables) {
    result = result.replaceAll(`@${variable.key}`, variable.value);
  }
  return result;
}

export interface EventEvaluationContext {
  readonly eventId: string;
  readonly nodeId: string;
  readonly variables: ReadonlyMap<string, string>;
}

export interface EventConditionEvaluation {
  readonly result: boolean;
  /** Opaque storage keys which are committed only after taking this branch. */
  readonly onceKeys?: readonly string[];
}

export interface EventRunnerCallbacks {
  evaluateCondition(
    expression: string,
    context: EventEvaluationContext,
  ): boolean | EventConditionEvaluation;
  execute(statement: string, context: EventEvaluationContext): void;
  commitOnceKeys?(
    keys: readonly string[],
    context: EventEvaluationContext,
  ): void;
}

export interface EventRunnerChoice {
  readonly authoredIndex: number;
  readonly key: string;
  readonly prefix: string;
  readonly text: string;
  readonly next: string;
  readonly onceKeys: readonly string[];
}

export type EventRunnerErrorCode =
  | 'not-started'
  | 'invalid-action'
  | 'invalid-choice'
  | 'missing-node'
  | 'duplicate-node'
  | 'unsupported-node'
  | 'condition-error'
  | 'execution-error'
  | 'step-limit';

export interface EventRunnerError {
  readonly code: EventRunnerErrorCode;
  readonly message: string;
  readonly nodeId?: string;
}

interface EventRunnerOutcomeBase {
  readonly nodeId: string;
  /** Nodes traversed by the command which produced this outcome. */
  readonly steps: number;
}

export type EventRunnerOutcome =
  | ({ readonly kind: 'idle' } & EventRunnerOutcomeBase)
  | ({
      readonly kind: 'continue';
      readonly text: string;
    } & EventRunnerOutcomeBase)
  | ({
      readonly kind: 'choice';
      readonly text: string;
      readonly choices: readonly EventRunnerChoice[];
    } & EventRunnerOutcomeBase)
  | ({ readonly kind: 'ended'; readonly text: string } & EventRunnerOutcomeBase)
  | ({
      readonly kind: 'error';
      readonly error: EventRunnerError;
    } & EventRunnerOutcomeBase);

export interface EventRunnerOptions {
  readonly maxSteps?: number;
}

export interface EventRunValidation {
  readonly runnable: boolean;
  readonly graphIssues: ReturnType<typeof analyzeEventGraph>['issues'];
  readonly variablePlan: EventVariablePlan;
}

export function validateEventRun(
  event: SpecialEventRecord,
  importedEvents: readonly SpecialEventRecord[] = [],
): EventRunValidation {
  const graphIssues = analyzeEventGraph(event).issues;
  return Object.freeze({
    runnable: !graphIssues.some(({ severity }) => severity === 'error'),
    graphIssues,
    variablePlan: resolveEventVariables(event, importedEvents),
  });
}

interface PendingChoice extends EventRunnerChoice {
  readonly execStr: string;
}

function normalizeCondition(
  evaluation: boolean | EventConditionEvaluation,
): EventConditionEvaluation {
  return typeof evaluation === 'boolean' ? { result: evaluation } : evaluation;
}

/** Split authored commands on newlines/semicolons outside parentheses. */
export function splitEventStatements(source: string): readonly string[] {
  const statements: string[] = [];
  let current = '';
  let depth = 0;
  for (const character of source) {
    if (character === '(') depth += 1;
    if (character === ')') depth -= 1;
    if ((character === ';' || character === '\n') && depth === 0) {
      if (current.trim() !== '') statements.push(current.trim());
      current = '';
    } else {
      current += character;
    }
  }
  if (current.trim() !== '') statements.push(current.trim());
  return Object.freeze(statements);
}

/**
 * Deterministic, DOM-free preview runner. It delegates the game's expression
 * language and state mutation to callbacks, while owning graph traversal,
 * variable substitution, branch order, and runaway-loop protection.
 */
export class EventRunner {
  readonly #event: SpecialEventRecord;
  readonly #nodes = new Map<string, EventNode[]>();
  readonly #callbacks: EventRunnerCallbacks;
  readonly #variables: EventVariablePlan;
  readonly #maxSteps: number;
  #outcome: EventRunnerOutcome = { kind: 'idle', nodeId: '', steps: 0 };
  #pendingChoices: readonly PendingChoice[] = [];
  #pendingNext = '';
  #queuedText = '';

  constructor(
    event: SpecialEventRecord,
    importedEvents: readonly SpecialEventRecord[],
    callbacks: EventRunnerCallbacks,
    options: EventRunnerOptions = {},
  ) {
    this.#event = parseSpecialEventRecord(event);
    this.#callbacks = callbacks;
    this.#variables = resolveEventVariables(this.#event, importedEvents);
    const maxSteps = options.maxSteps ?? 500;
    if (!Number.isSafeInteger(maxSteps) || maxSteps < 1) {
      throw new RangeError('event runner maxSteps must be a positive integer');
    }
    this.#maxSteps = maxSteps;
    for (const node of this.#event.children ?? []) {
      const matches = this.#nodes.get(node.id) ?? [];
      matches.push(node);
      this.#nodes.set(node.id, matches);
    }
  }

  get variablePlan(): EventVariablePlan {
    return this.#variables;
  }

  get outcome(): EventRunnerOutcome {
    return this.#outcome;
  }

  reset(): EventRunnerOutcome {
    this.#pendingChoices = [];
    this.#pendingNext = '';
    this.#queuedText = '';
    this.#outcome = { kind: 'idle', nodeId: '', steps: 0 };
    return this.#outcome;
  }

  start(): EventRunnerOutcome {
    this.reset();
    const children = this.#event.children ?? [];
    const entry = this.#nodes.has('root') ? 'root' : (children[0]?.id ?? '');
    if (entry === '') {
      return this.#fail('missing-node', 'The event has no entry node.', '', 0);
    }
    return this.#advance(entry);
  }

  continue(): EventRunnerOutcome {
    if (this.#outcome.kind !== 'continue') {
      return this.#fail(
        'invalid-action',
        'The current event outcome cannot be continued.',
        this.#outcome.nodeId,
        0,
      );
    }
    return this.#advance(this.#pendingNext);
  }

  selectChoice(visibleChoiceIndex: number): EventRunnerOutcome {
    if (this.#outcome.kind !== 'choice') {
      return this.#fail(
        'invalid-action',
        'The current event outcome has no selectable choices.',
        this.#outcome.nodeId,
        0,
      );
    }
    if (
      !Number.isSafeInteger(visibleChoiceIndex) ||
      visibleChoiceIndex < 0 ||
      visibleChoiceIndex >= this.#pendingChoices.length
    ) {
      return this.#fail(
        'invalid-choice',
        `Choice index ${visibleChoiceIndex} is out of bounds.`,
        this.#outcome.nodeId,
        0,
      );
    }

    const choice = this.#pendingChoices[visibleChoiceIndex]!;
    const context = this.#context(this.#outcome.nodeId);
    try {
      if (choice.onceKeys.length > 0) {
        this.#callbacks.commitOnceKeys?.(choice.onceKeys, context);
      }
      this.#execute(choice.execStr, context);
    } catch (error) {
      return this.#fail(
        'execution-error',
        error instanceof Error ? error.message : String(error),
        context.nodeId,
        0,
      );
    }
    return this.#advance(choice.next);
  }

  #advance(initialNodeId: string): EventRunnerOutcome {
    let nodeId = initialNodeId;
    let steps = 0;
    this.#pendingChoices = [];
    this.#pendingNext = '';

    while (true) {
      if (steps >= this.#maxSteps) {
        return this.#fail(
          'step-limit',
          `Event traversal exceeded ${this.#maxSteps} steps.`,
          nodeId,
          steps,
        );
      }
      steps += 1;
      const matches = this.#nodes.get(nodeId);
      if (matches === undefined) {
        return this.#fail(
          'missing-node',
          `Event node "${nodeId}" does not exist.`,
          nodeId,
          steps,
        );
      }
      if (matches.length !== 1) {
        return this.#fail(
          'duplicate-node',
          `${matches.length} event nodes use ID "${nodeId}".`,
          nodeId,
          steps,
        );
      }

      const node = matches[0]!;
      const context = this.#context(node.id);
      if (node.eventChildType === 'EXEC') {
        const execNode = node as EventExecNode;
        try {
          this.#execute(execNode.execStr ?? '', context);
        } catch (error) {
          return this.#fail(
            'execution-error',
            error instanceof Error ? error.message : String(error),
            node.id,
            steps,
          );
        }
        const text = this.#replace(execNode.p ?? '');
        const autoAdvance =
          text === '' ||
          (this.#event.eventType !== 'MODAL' && execNode.autoAdvance === true);
        if (autoAdvance) {
          this.#queuedText = joinText(this.#queuedText, text);
          nodeId = execNode.next ?? '';
          continue;
        }
        this.#pendingNext = execNode.next ?? '';
        const displayText = joinText(this.#queuedText, text);
        this.#queuedText = '';
        return this.#setOutcome({
          kind: 'continue',
          nodeId: node.id,
          text: displayText,
          steps,
        });
      }

      if (node.eventChildType === 'CHOICE') {
        const choiceNode = node as EventChoiceNode;
        const choices: PendingChoice[] = [];
        for (const [authoredIndex, choice] of (
          choiceNode.choices ?? []
        ).entries()) {
          const condition = choice.conditionStr ?? '';
          const evaluation = this.#condition(condition, context);
          if (evaluation instanceof Error) {
            return this.#fail(
              'condition-error',
              evaluation.message,
              node.id,
              steps,
            );
          }
          if (!evaluation.result) continue;
          const resolvedText = this.#choiceText(choice, context);
          if (resolvedText instanceof Error) {
            return this.#fail(
              'condition-error',
              resolvedText.message,
              node.id,
              steps,
            );
          }
          const onceKeys = [
            ...(evaluation.onceKeys ?? []),
            ...resolvedText.onceKeys,
          ];
          choices.push(
            Object.freeze({
              authoredIndex,
              key: `${node.id}:${authoredIndex}`,
              prefix: this.#replace(choice.prefixText ?? ''),
              text: resolvedText.text,
              next: choice.next ?? '',
              execStr: choice.evalStr ?? '',
              onceKeys: Object.freeze(onceKeys),
            }),
          );
        }
        const text = joinText(
          this.#queuedText,
          this.#replace(choiceNode.text ?? ''),
        );
        this.#queuedText = '';
        this.#pendingChoices = choices;
        return this.#setOutcome({
          kind: 'choice',
          nodeId: node.id,
          text,
          choices: Object.freeze(
            choices.map((choice) => ({
              authoredIndex: choice.authoredIndex,
              key: choice.key,
              prefix: choice.prefix,
              text: choice.text,
              next: choice.next,
              onceKeys: choice.onceKeys,
            })),
          ),
          steps,
        });
      }

      if (node.eventChildType === 'SWITCH') {
        const switchNode = node as EventSwitchNode;
        let next = switchNode.defaultNext ?? '';
        for (const switchCase of switchNode.cases ?? []) {
          const evaluation = this.#condition(
            switchCase.conditionStr ?? '',
            context,
          );
          if (evaluation instanceof Error) {
            return this.#fail(
              'condition-error',
              evaluation.message,
              node.id,
              steps,
            );
          }
          if (!evaluation.result) continue;
          try {
            this.#callbacks.commitOnceKeys?.(
              evaluation.onceKeys ?? [],
              context,
            );
          } catch (error) {
            return this.#fail(
              'execution-error',
              error instanceof Error ? error.message : String(error),
              node.id,
              steps,
            );
          }
          next = switchCase.next ?? '';
          break;
        }
        nodeId = next;
        continue;
      }

      if (node.eventChildType === 'END') {
        const text =
          this.#event.eventType === 'TALK'
            ? ''
            : joinText(this.#queuedText, 'End.');
        this.#queuedText = '';
        return this.#setOutcome({
          kind: 'ended',
          nodeId: node.id,
          text,
          steps,
        });
      }

      return this.#fail(
        'unsupported-node',
        `Event node "${node.id}" has unsupported type "${node.eventChildType}".`,
        node.id,
        steps,
      );
    }
  }

  #choiceText(
    choice: EventChoice,
    context: EventEvaluationContext,
  ): { readonly text: string; readonly onceKeys: readonly string[] } | Error {
    for (const alternate of choice.switchText ?? []) {
      const condition = alternate.conditionStr ?? '';
      if (condition === '') continue;
      const evaluation = this.#condition(condition, context);
      if (evaluation instanceof Error) return evaluation;
      if (evaluation.result) {
        return {
          text: this.#replace(alternate.text ?? ''),
          onceKeys: evaluation.onceKeys ?? [],
        };
      }
    }
    return { text: this.#replace(choice.text ?? ''), onceKeys: [] };
  }

  #condition(
    expression: string,
    context: EventEvaluationContext,
  ): EventConditionEvaluation | Error {
    if (expression === '') return { result: true };
    const resolved = this.#replace(expression);
    try {
      return normalizeCondition(
        this.#callbacks.evaluateCondition(resolved, context),
      );
    } catch (error) {
      const detail = error instanceof Error ? error.message : String(error);
      return new Error(`Condition "${resolved}" failed: ${detail}`, {
        cause: error,
      });
    }
  }

  #execute(source: string, context: EventEvaluationContext): void {
    for (const statement of splitEventStatements(source)) {
      const resolved = this.#replace(statement);
      try {
        this.#callbacks.execute(resolved, context);
      } catch (error) {
        const detail = error instanceof Error ? error.message : String(error);
        throw new Error(`Statement "${resolved}" failed: ${detail}`, {
          cause: error,
        });
      }
    }
  }

  #replace(text: string): string {
    return replaceEventVariables(text, this.#variables);
  }

  #context(nodeId: string): EventEvaluationContext {
    return Object.freeze({
      eventId: this.#event.id,
      nodeId,
      variables: this.#variables.values,
    });
  }

  #setOutcome(outcome: EventRunnerOutcome): EventRunnerOutcome {
    this.#outcome = Object.freeze(outcome);
    return this.#outcome;
  }

  #fail(
    code: EventRunnerErrorCode,
    message: string,
    nodeId: string,
    steps: number,
  ): EventRunnerOutcome {
    return this.#setOutcome({
      kind: 'error',
      nodeId,
      steps,
      error: Object.freeze({ code, message, nodeId: nodeId || undefined }),
    });
  }
}

function joinText(earlier: string, later: string): string {
  if (earlier === '') return later;
  if (later === '') return earlier;
  return `${earlier}\n\n${later}`;
}

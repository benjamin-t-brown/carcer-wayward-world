import {
  KNOWN_EVENT_NODE_TYPES,
  RUNTIME_EVENT_NODE_TYPES,
  type EventChoice,
  type EventConnection,
  type EventGraphAnalysis,
  type EventGraphIssue,
  type EventNode,
  type EventSwitchCase,
  type SpecialEventRecord,
} from './types.js';

function optionalTarget(value: unknown): string | undefined {
  return typeof value === 'string' && value !== '' ? value : undefined;
}

/** Enumerate runtime graph exits in their persisted branch order. */
export function connectionsFromNode(
  node: EventNode,
): readonly EventConnection[] {
  const connections: EventConnection[] = [];
  const add = (
    kind: EventConnection['kind'],
    target: unknown,
    branchIndex?: number,
  ) => {
    const targetId = optionalTarget(target);
    if (targetId === undefined) return;
    connections.push(
      branchIndex === undefined
        ? { sourceId: node.id, targetId, kind }
        : { sourceId: node.id, targetId, kind, branchIndex },
    );
  };

  if (node.eventChildType === 'EXEC' || node.eventChildType === 'END') {
    add('next', node.next);
  } else if (node.eventChildType === 'SWITCH') {
    const cases = Array.isArray(node.cases)
      ? (node.cases as EventSwitchCase[])
      : [];
    cases.forEach((branch, index) => add('switchCase', branch.next, index));
    add('defaultNext', node.defaultNext);
  } else if (node.eventChildType === 'CHOICE') {
    const choices = Array.isArray(node.choices)
      ? (node.choices as EventChoice[])
      : [];
    choices.forEach((branch, index) => add('choice', branch.next, index));
  }
  return Object.freeze(connections);
}

function frozenMapOfArrays<T>(
  source: Map<string, T[]>,
): ReadonlyMap<string, readonly T[]> {
  return new Map(
    [...source].map(([key, values]) => [key, Object.freeze(values)]),
  );
}

/**
 * Build indexes and graph diagnostics without rewriting any event data. Unknown
 * node kinds stay in the node index but intentionally contribute no guessed
 * connectors.
 */
export function analyzeEventGraph(
  event: SpecialEventRecord,
): EventGraphAnalysis {
  const children = event.children ?? [];
  const mutableNodes = new Map<string, EventNode[]>();
  const normalizedIds = new Map<string, EventNode[]>();
  const issues: EventGraphIssue[] = [];

  for (const node of children) {
    const sameId = mutableNodes.get(node.id) ?? [];
    sameId.push(node);
    mutableNodes.set(node.id, sameId);

    const normalizedId = node.id.trim();
    const sameNormalizedId = normalizedIds.get(normalizedId) ?? [];
    sameNormalizedId.push(node);
    normalizedIds.set(normalizedId, sameNormalizedId);

    if (!KNOWN_EVENT_NODE_TYPES.includes(node.eventChildType as never)) {
      issues.push({
        severity: 'warning',
        code: 'unknown-node-type',
        nodeId: node.id,
        message: `Node "${node.id}" has unknown type "${node.eventChildType}".`,
      });
    } else if (
      !RUNTIME_EVENT_NODE_TYPES.includes(node.eventChildType as never)
    ) {
      issues.push({
        severity: 'warning',
        code: 'runtime-unsupported-node-type',
        nodeId: node.id,
        message: `Node "${node.id}" (${node.eventChildType}) is preserved but skipped by the game runtime.`,
      });
    }
  }

  for (const [normalizedId, nodes] of normalizedIds) {
    if (nodes.length > 1) {
      issues.push({
        severity: 'error',
        code: 'duplicate-node-id',
        nodeId: normalizedId,
        message: `${nodes.length} nodes share the ID "${normalizedId}" after trimming.`,
      });
    }
  }

  const connections = children.flatMap((node) => connectionsFromNode(node));
  const mutableIncoming = new Map<string, EventConnection[]>();
  for (const connection of connections) {
    const incoming = mutableIncoming.get(connection.targetId) ?? [];
    incoming.push(connection);
    mutableIncoming.set(connection.targetId, incoming);
    if (!mutableNodes.has(connection.targetId)) {
      issues.push({
        severity: 'error',
        code: 'missing-target',
        nodeId: connection.sourceId,
        targetId: connection.targetId,
        message: `Node "${connection.sourceId}" links to missing node "${connection.targetId}".`,
      });
    }
  }

  const entryId = mutableNodes.has('root') ? 'root' : children[0]?.id;
  if (children.length > 0 && !mutableNodes.has('root')) {
    issues.push({
      severity: 'warning',
      code: 'missing-root',
      message:
        'The event has no node with ID "root"; the first node is the fallback entry.',
    });
  }

  const outgoingByNodeId = new Map<string, EventConnection[]>();
  for (const connection of connections) {
    const outgoing = outgoingByNodeId.get(connection.sourceId) ?? [];
    outgoing.push(connection);
    outgoingByNodeId.set(connection.sourceId, outgoing);
  }
  const reachable = new Set<string>();
  const pending = entryId === undefined ? [] : [entryId];
  while (pending.length > 0) {
    const nodeId = pending.pop()!;
    if (reachable.has(nodeId) || !mutableNodes.has(nodeId)) continue;
    reachable.add(nodeId);
    for (const connection of outgoingByNodeId.get(nodeId) ?? []) {
      pending.push(connection.targetId);
    }
  }
  for (const node of children) {
    if (node.eventChildType !== 'COMMENT' && !reachable.has(node.id)) {
      issues.push({
        severity: 'warning',
        code: 'unreachable-node',
        nodeId: node.id,
        message: `Node "${node.id}" cannot be reached from the event entry.`,
      });
    }
  }

  return Object.freeze({
    nodesById: frozenMapOfArrays(mutableNodes),
    connections: Object.freeze(connections),
    incomingByNodeId: frozenMapOfArrays(mutableIncoming),
    reachableNodeIds: reachable,
    issues: Object.freeze(issues),
  });
}

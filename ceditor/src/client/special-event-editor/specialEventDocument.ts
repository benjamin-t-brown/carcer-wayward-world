import type { GameEvent, SENode } from '../types/assets';

/** Minimal boundary implemented by every canvas editor node. */
export interface SpecialEventNodeSerializer {
  toSENode(): SENode;
}

export interface ActiveSpecialEventDocument {
  eventId: string;
  nodes: readonly SpecialEventNodeSerializer[];
}

export interface SpecialEventDocumentSnapshot {
  /** Complete, current event collection suitable for save or reference lookup. */
  gameEvents: GameEvent[];
  /** Independent copy for validation, JSON output, or the event runner. */
  currentGameEvent?: GameEvent;
}

/** Detach an asset document before constructing mutable editor-node state. */
export function cloneSpecialEventDocument(gameEvent: GameEvent): GameEvent {
  return structuredClone(gameEvent);
}

/**
 * Serialize editor nodes without retaining references to mutable editor state.
 *
 * Some node implementations contain nested arrays and objects. Cloning the
 * completed serialization, instead of relying on each implementation to clone
 * every field, keeps that implementation detail out of persistence.
 */
export function serializeSpecialEventNodes(
  nodes: readonly SpecialEventNodeSerializer[],
): SENode[] {
  return structuredClone(nodes.map((node) => node.toSENode()));
}

/** Build one detached event document from its asset metadata and editor nodes. */
export function serializeSpecialEventDocument(
  gameEvent: GameEvent,
  nodes: readonly SpecialEventNodeSerializer[],
): GameEvent {
  const document = cloneSpecialEventDocument(gameEvent);
  document.children = serializeSpecialEventNodes(nodes);
  return document;
}

/**
 * Take the single document snapshot used when switching, saving, validating,
 * displaying JSON, or starting the runner.
 *
 * The inputs, returned collection, and returned current event share no mutable
 * objects. A stale active id therefore cannot mutate an unrelated asset, and a
 * consumer such as the runner cannot change the collection prepared for save.
 */
export function snapshotSpecialEventDocuments(
  gameEvents: readonly GameEvent[],
  active?: ActiveSpecialEventDocument,
): SpecialEventDocumentSnapshot {
  const documents = structuredClone(gameEvents) as GameEvent[];
  if (!active) {
    return { gameEvents: documents };
  }

  const activeIndex = documents.findIndex(
    (gameEvent) => gameEvent.id === active.eventId,
  );
  if (activeIndex === -1) {
    return { gameEvents: documents };
  }

  documents[activeIndex] = serializeSpecialEventDocument(
    documents[activeIndex],
    active.nodes,
  );

  return {
    gameEvents: documents,
    currentGameEvent: structuredClone(documents[activeIndex]),
  };
}

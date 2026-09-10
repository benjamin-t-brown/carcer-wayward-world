import type { HistoryCommand } from './cellPatches.js';

export const DEFAULT_HISTORY_LIMIT = 100;

/**
 * Count-bounded undo/redo storage. Commands own their small patches; the
 * history never snapshots a document.
 */
export class BoundedHistory<Target> {
  private readonly past: HistoryCommand<Target>[] = [];
  private readonly future: HistoryCommand<Target>[] = [];

  constructor(readonly maxEntries = DEFAULT_HISTORY_LIMIT) {
    if (!Number.isInteger(maxEntries) || maxEntries < 1) {
      throw new RangeError('History limit must be a positive integer');
    }
  }

  get undoDepth(): number {
    return this.past.length;
  }

  get redoDepth(): number {
    return this.future.length;
  }

  get canUndo(): boolean {
    return this.past.length > 0;
  }

  get canRedo(): boolean {
    return this.future.length > 0;
  }

  /** Record a command whose edits were applied interactively by a gesture. */
  recordApplied(command: HistoryCommand<Target>): boolean {
    if (command.isEmpty) {
      return false;
    }
    this.past.push(command);
    this.future.length = 0;
    if (this.past.length > this.maxEntries) {
      this.past.splice(0, this.past.length - this.maxEntries);
    }
    return true;
  }

  /** Apply a complete command now and add it to history. */
  execute(command: HistoryCommand<Target>, target: Target): boolean {
    if (command.isEmpty) {
      return false;
    }
    command.redo(target);
    return this.recordApplied(command);
  }

  undo(target: Target): HistoryCommand<Target> | undefined {
    const command = this.past.at(-1);
    if (!command) {
      return undefined;
    }
    command.undo(target);
    this.past.pop();
    this.future.push(command);
    return command;
  }

  redo(target: Target): HistoryCommand<Target> | undefined {
    const command = this.future.at(-1);
    if (!command) {
      return undefined;
    }
    command.redo(target);
    this.future.pop();
    this.past.push(command);
    return command;
  }

  clear(): void {
    this.past.length = 0;
    this.future.length = 0;
  }
}

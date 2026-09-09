import { CarcerMapTemplate } from '../../types/assets';
import { EditorState } from '../editorState';
import type { PaintAction } from '../paintTools';

/**
 * A map editing tool. Replaces the per-member branches that were spread across
 * applyAction / applyActionUpdate / undoAction in paintTools.ts, the tool grid
 * in MapToolsOverlay.tsx, and the shortcut block in editorEvents.ts.
 *
 * `id` is the tool's PaintActionType string value, so the registry and the
 * legacy enum stay interchangeable.
 */
export interface MapTool {
  id: string;
  /** Emoji shown in the overlay. */
  icon: string;
  /** Tooltip, e.g. 'Terrain tool (T)'. */
  title: string;
  /** Single key that selects the tool, e.g. 't'. */
  shortcut?: string;
  /**
   * Whether the shortcut is ignored while Ctrl is held. Preserves the existing
   * split: b/f select their tool even with Ctrl, e/s/c/t do not (so Ctrl+S
   * stays "save", etc.).
   */
  shortcutBlockedByCtrl?: boolean;
  /** Extra class on the icon span (the delete-fill bucket variant). */
  iconClassName?: string;
  /** Which overlay row the button sits in. */
  row: 'primary' | 'secondary';

  /** Mutate the working tile buffer when a stroke starts / completes. */
  apply(action: PaintAction, map: CarcerMapTemplate, s: EditorState): void;
  /** Mutate the working tile buffer for each tile entered during a drag. */
  update?(action: PaintAction, map: CarcerMapTemplate, s: EditorState): void;
  /** Restore the tiles this action touched. Every tool must be undoable. */
  undo(action: PaintAction, map: CarcerMapTemplate): void;
}

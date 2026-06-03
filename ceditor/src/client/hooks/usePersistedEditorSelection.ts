import { useEffect, useLayoutEffect, useRef } from 'react';
import {
  EditorSelectionKey,
  loadEditorSelection,
  resolveSelectionFromRoute,
  saveEditorSelection,
} from '../utils/editorSelectionStorage';

interface UsePersistedEditorSelectionOptions<T> {
  editorKey: EditorSelectionKey;
  items: T[];
  getId: (item: T) => string;
  selectedIndex: number;
  setSelectedIndex: (index: number) => void;
  routeParams?: URLSearchParams;
  /** Called after restoring a selection (e.g. scroll form into view). */
  onRestored?: (index: number) => void;
}

/**
 * Persists the selected list item in localStorage and restores it when the
 * editor page is opened again. Hash query params take precedence over storage.
 */
export function usePersistedEditorSelection<T>({
  editorKey,
  items,
  getId,
  selectedIndex,
  setSelectedIndex,
  routeParams,
  onRestored,
}: UsePersistedEditorSelectionOptions<T>): void {
  const hasRestoredRef = useRef(false);
  const isFirstPersistRef = useRef(true);
  const hydratedRef = useRef(false);

  useLayoutEffect(() => {
    if (hasRestoredRef.current || items.length === 0) {
      return;
    }
    hasRestoredRef.current = true;

    const entityId =
      resolveSelectionFromRoute(editorKey, routeParams) ??
      loadEditorSelection(editorKey);

    if (entityId) {
      const index = items.findIndex((item) => getId(item) === entityId);
      if (index >= 0) {
        setSelectedIndex(index);
        onRestored?.(index);
      }
    }

    hydratedRef.current = true;
  }, [editorKey, items, routeParams, getId, setSelectedIndex, onRestored]);

  useEffect(() => {
    if (!hydratedRef.current) {
      return;
    }

    if (isFirstPersistRef.current) {
      isFirstPersistRef.current = false;
      if (selectedIndex < 0 && loadEditorSelection(editorKey)) {
        return;
      }
    }

    if (selectedIndex >= 0 && selectedIndex < items.length) {
      saveEditorSelection(editorKey, getId(items[selectedIndex]));
    } else {
      saveEditorSelection(editorKey, null);
    }
  }, [editorKey, items, selectedIndex, getId]);
}

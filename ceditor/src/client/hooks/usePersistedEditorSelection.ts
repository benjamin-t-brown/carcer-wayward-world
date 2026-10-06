import { useEffect, useLayoutEffect, useRef } from 'react';
import {
  EditorSelectionKey,
  loadEditorSelection,
  resolveSelectionFromRoute,
  saveEditorSelection,
  syncSelectionToRoute,
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
 * editor page is opened again. Hash query params take precedence over storage,
 * stay in sync with the selection, and participate in browser history.
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
  const lastRouteEntityIdRef = useRef<string | null | undefined>(undefined);
  const preferReplaceHistoryRef = useRef(true);
  const selectedIndexRef = useRef(selectedIndex);
  const skipNextRouteSyncRef = useRef(false);

  selectedIndexRef.current = selectedIndex;

  useLayoutEffect(() => {
    if (hasRestoredRef.current || items.length === 0) {
      return;
    }
    hasRestoredRef.current = true;

    const routeEntityId = resolveSelectionFromRoute(editorKey, routeParams);
    lastRouteEntityIdRef.current = routeEntityId;

    const entityId = routeEntityId ?? loadEditorSelection(editorKey);

    if (entityId) {
      const index = items.findIndex((item) => getId(item) === entityId);
      if (index >= 0) {
        setSelectedIndex(index);
        onRestored?.(index);
      }
    }

    hydratedRef.current = true;
  }, [editorKey, items, routeParams, getId, setSelectedIndex, onRestored]);

  // Apply selection when the hash query changes (browser back/forward).
  useEffect(() => {
    // Without live route params we cannot tell clear vs. missing wiring; do not
    // clobber the current selection.
    if (!hydratedRef.current || items.length === 0 || !routeParams) {
      return;
    }

    const routeEntityId = resolveSelectionFromRoute(editorKey, routeParams);
    if (routeEntityId === lastRouteEntityIdRef.current) {
      return;
    }
    lastRouteEntityIdRef.current = routeEntityId;

    if (routeEntityId) {
      const index = items.findIndex((item) => getId(item) === routeEntityId);
      if (index >= 0 && index !== selectedIndexRef.current) {
        // Selection follows the URL; do not push the stale selection back.
        skipNextRouteSyncRef.current = true;
        setSelectedIndex(index);
        onRestored?.(index);
      }
      return;
    }

    if (selectedIndexRef.current >= 0) {
      skipNextRouteSyncRef.current = true;
      setSelectedIndex(-1);
    }
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

    const entityId =
      selectedIndex >= 0 && selectedIndex < items.length
        ? getId(items[selectedIndex])
        : null;

    saveEditorSelection(editorKey, entityId);

    if (skipNextRouteSyncRef.current) {
      skipNextRouteSyncRef.current = false;
      return;
    }

    const historyMode = preferReplaceHistoryRef.current ? 'replace' : 'push';
    preferReplaceHistoryRef.current = false;
    syncSelectionToRoute(editorKey, entityId, { history: historyMode });
    lastRouteEntityIdRef.current = entityId;
  }, [editorKey, items, selectedIndex, getId]);
}

import { useEffect, useLayoutEffect, useRef } from 'react';
import {
  EditorSelectionKey,
  loadEditorSelection,
  resolveSelectionFromRoute,
  saveEditorSelection,
  syncSelectionToRoute,
} from '../utils/editorSelectionStorage';
import { readHashRoute } from '../utils/hashRoute';

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

function routeEntityFor(
  editorKey: EditorSelectionKey,
  routeParams?: URLSearchParams,
): string | null {
  return (
    resolveSelectionFromRoute(editorKey, routeParams) ??
    // Props can lag one frame behind the hash after <a href="#..."> navigation.
    resolveSelectionFromRoute(editorKey, readHashRoute().params)
  );
}

/**
 * Persists the selected list item in localStorage and restores it when the
 * editor page is opened again. Hash query params take precedence over storage
 * and stay in sync with the selection via history.replaceState (so deep-links
 * and in-editor selection changes do not stack extra history entries).
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
  const hydratedRef = useRef(false);
  const lastRouteEntityIdRef = useRef<string | null | undefined>(undefined);
  const selectedIndexRef = useRef(selectedIndex);
  const skipNextRouteSyncRef = useRef(false);
  const getIdRef = useRef(getId);
  const onRestoredRef = useRef(onRestored);

  selectedIndexRef.current = selectedIndex;
  getIdRef.current = getId;
  onRestoredRef.current = onRestored;

  useLayoutEffect(() => {
    if (hasRestoredRef.current || items.length === 0) {
      return;
    }
    hasRestoredRef.current = true;

    const routeEntityId = routeEntityFor(editorKey, routeParams);
    lastRouteEntityIdRef.current = routeEntityId;

    const entityId = routeEntityId ?? loadEditorSelection(editorKey);

    if (entityId) {
      const index = items.findIndex(
        (item) => getIdRef.current(item) === entityId,
      );
      if (index >= 0) {
        setSelectedIndex(index);
        onRestoredRef.current?.(index);
      }
    }

    hydratedRef.current = true;
  }, [editorKey, items, routeParams, setSelectedIndex]);

  // Apply selection when the hash query changes (browser back/forward).
  useEffect(() => {
    if (!hydratedRef.current || items.length === 0) {
      return;
    }

    const routeEntityId = routeEntityFor(editorKey, routeParams);
    if (routeEntityId === lastRouteEntityIdRef.current) {
      return;
    }
    lastRouteEntityIdRef.current = routeEntityId;

    if (routeEntityId) {
      const index = items.findIndex(
        (item) => getIdRef.current(item) === routeEntityId,
      );
      if (index >= 0 && index !== selectedIndexRef.current) {
        skipNextRouteSyncRef.current = true;
        setSelectedIndex(index);
        onRestoredRef.current?.(index);
      }
      return;
    }

    if (selectedIndexRef.current >= 0) {
      skipNextRouteSyncRef.current = true;
      setSelectedIndex(-1);
    }
  }, [editorKey, items, routeParams, setSelectedIndex]);

  useEffect(() => {
    if (!hydratedRef.current) {
      return;
    }

    const entityId =
      selectedIndex >= 0 && selectedIndex < items.length
        ? getIdRef.current(items[selectedIndex])
        : null;

    saveEditorSelection(editorKey, entityId);

    if (skipNextRouteSyncRef.current) {
      skipNextRouteSyncRef.current = false;
      return;
    }

    // Selection may still be empty for a frame after a deep-link mount. Never
    // strip ?ability= / ?spell= from the live hash in that window.
    if (entityId == null) {
      const live = resolveSelectionFromRoute(
        editorKey,
        readHashRoute().params,
      );
      if (live) {
        return;
      }
    }

    syncSelectionToRoute(editorKey, entityId, { history: 'replace' });
    lastRouteEntityIdRef.current = entityId;
  }, [editorKey, items, selectedIndex]);
}

import { useEffect, useRef } from 'react';
import { isEditorModalOpen, isTextInputElement } from '../utils/editorHotkeys';

/** Binds Alt+R to reload asset lists this page is not currently editing. */
export function useReloadAssetsHotkey(onReload: (() => void) | undefined): void {
  const onReloadRef = useRef(onReload);
  onReloadRef.current = onReload;

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (!onReloadRef.current) {
        return;
      }
      if (
        !event.altKey ||
        event.ctrlKey ||
        event.metaKey ||
        event.shiftKey ||
        event.code !== 'KeyR'
      ) {
        return;
      }
      if (isTextInputElement(document.activeElement) || isEditorModalOpen()) {
        return;
      }
      event.preventDefault();
      event.stopPropagation();
      onReloadRef.current();
    };
    window.addEventListener('keydown', handleKeyDown, true);
    return () => window.removeEventListener('keydown', handleKeyDown, true);
  }, []);
}

import { useEffect, useRef } from 'react';

export const MODAL_ROOT_CLASS = 'generic-modal';

export function getTopmostModalRoot(): HTMLElement | null {
  const modals = Array.from(
    document.querySelectorAll<HTMLElement>(`.${MODAL_ROOT_CLASS}`)
  );
  if (modals.length === 0) {
    return null;
  }

  let top: HTMLElement | null = null;
  let topZ = Number.NEGATIVE_INFINITY;
  for (const modal of modals) {
    const z = Number.parseInt(window.getComputedStyle(modal).zIndex, 10);
    const zIndex = Number.isFinite(z) ? z : 0;
    if (zIndex >= topZ) {
      topZ = zIndex;
      top = modal;
    }
  }
  return top;
}

export function useEscapeToClose(onClose: () => void, enabled = true) {
  const modalRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!enabled) {
      return;
    }

    const handleKeyDown = (event: KeyboardEvent) => {
      if (event.key !== 'Escape') {
        return;
      }

      const topmost = getTopmostModalRoot();
      if (!modalRef.current || modalRef.current !== topmost) {
        return;
      }

      event.preventDefault();
      event.stopPropagation();
      onClose();
    };

    window.addEventListener('keydown', handleKeyDown, true);
    return () => window.removeEventListener('keydown', handleKeyDown, true);
  }, [onClose, enabled]);

  return modalRef;
}

export function isTextInputElement(el: Element | null): boolean {
  if (!el || !(el instanceof HTMLElement)) {
    return false;
  }
  const tag = el.tagName;
  return (
    tag === 'INPUT' ||
    tag === 'TEXTAREA' ||
    tag === 'SELECT' ||
    el.isContentEditable
  );
}

export function isEditorModalOpen(): boolean {
  return document.querySelector('.generic-modal') !== null;
}

export function isSpacePanKey(ev: KeyboardEvent): boolean {
  return ev.code === 'Space' || ev.key === ' ' || ev.key === 'Spacebar';
}

export function isTileCopyKey(ev: KeyboardEvent): boolean {
  return ev.key === 'q' || ev.key === 'Q';
}

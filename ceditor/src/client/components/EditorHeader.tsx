import type { ReactNode } from 'react';
import { Button } from '../elements/Button';

interface EditorHeaderProps {
  title: string;
  showBack?: boolean;
  backHref?: string;
  backLabel?: string;
  onSave?: () => void;
  saveLabel?: string;
  /** Extra controls between the title and Save (e.g. map picker, New Map). */
  actions?: ReactNode;
}

export function EditorHeader({
  title,
  showBack = true,
  backHref = '#/',
  backLabel = '← Back',
  onSave,
  saveLabel = 'Save All',
  actions,
}: EditorHeaderProps) {
  return (
    <header className="editor-header">
      {showBack ? (
        <div className="editor-header-start">
          <Button variant="back" onClick={() => (window.location.hash = backHref)}>
            {backLabel}
          </Button>
        </div>
      ) : null}
      <h1 className="editor-header-title">{title}</h1>
      <div className="editor-header-actions">
        {actions}
        {onSave ? (
          <Button variant="primary" onClick={onSave}>
            {saveLabel}
          </Button>
        ) : null}
      </div>
    </header>
  );
}

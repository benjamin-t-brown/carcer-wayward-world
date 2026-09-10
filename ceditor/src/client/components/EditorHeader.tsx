import {
  useCallback,
  useEffect,
  useRef,
  useState,
  type ReactNode,
} from 'react';
import { useAssets } from '../contexts/AssetsContext';
import { Button } from '../elements/Button';

interface EditorHeaderProps {
  title: string;
  showBack?: boolean;
  backHref?: string;
  backLabel?: string;
  onSave?: () => void | Promise<void>;
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
  const { isSaving: isDatabaseSaving } = useAssets();
  const onSaveRef = useRef(onSave);
  const savingRef = useRef(false);
  const databaseSavingRef = useRef(isDatabaseSaving);
  const [isSaving, setIsSaving] = useState(false);
  const hasSaveAction = onSave !== undefined;

  onSaveRef.current = onSave;
  databaseSavingRef.current = isDatabaseSaving;

  const handleSave = useCallback(async () => {
    const save = onSaveRef.current;
    if (!save || savingRef.current || databaseSavingRef.current) {
      return;
    }

    savingRef.current = true;
    setIsSaving(true);
    try {
      await save();
    } catch (error) {
      console.error('CEditor save action failed:', error);
    } finally {
      savingRef.current = false;
      setIsSaving(false);
    }
  }, []);

  useEffect(() => {
    if (!hasSaveAction) {
      return;
    }

    const handleKeyDown = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 's') {
        event.preventDefault();
        void handleSave();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [handleSave, hasSaveAction]);

  const handleBack = () => {
    window.location.hash = backHref;
  };

  return (
    <header className="editor-header">
      {showBack ? (
        <div className="editor-header-start">
          <Button variant="back" onClick={handleBack}>
            {backLabel}
          </Button>
        </div>
      ) : null}
      <h1 className="editor-header-title">{title}</h1>
      <div className="editor-header-actions">
        {actions}
        {onSave ? (
          <Button
            variant="primary"
            onClick={() => void handleSave()}
            disabled={isSaving || isDatabaseSaving}
          >
            {saveLabel}
          </Button>
        ) : null}
      </div>
    </header>
  );
}

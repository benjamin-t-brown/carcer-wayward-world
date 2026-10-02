import { useEffect, useRef, useState, type ReactNode } from 'react';
import { Button } from '../elements/Button';
import { useAssets } from '../contexts/AssetsContext';
import { useReloadAssetsHotkey } from '../hooks/useReloadAssetsHotkey';
import type { AssetId } from '../../shared/assetRegistry';

interface EditorHeaderProps {
  title: string;
  showBack?: boolean;
  backHref?: string;
  backLabel?: string;
  onSave?: () => void;
  saveLabel?: string;
  /**
   * Asset list this page is editing. Reload refetches every other list from
   * disk and leaves this one alone, so unsaved work on the open page stays.
   */
  preserveAsset?: AssetId;
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
  preserveAsset,
  actions,
}: EditorHeaderProps) {
  const { reloadOtherAssets, loading } = useAssets();
  const [reloadNote, setReloadNote] = useState('');
  const reloadBusy = useRef(false);

  const handleReload = async () => {
    if (!preserveAsset || loading || reloadBusy.current) {
      return;
    }
    reloadBusy.current = true;
    setReloadNote('');
    try {
      await reloadOtherAssets(preserveAsset);
      setReloadNote('Reloaded');
    } catch {
      setReloadNote('Reload failed');
    } finally {
      reloadBusy.current = false;
    }
  };

  useReloadAssetsHotkey(preserveAsset ? handleReload : undefined);

  useEffect(() => {
    if (!reloadNote) {
      return;
    }
    const timer = window.setTimeout(() => setReloadNote(''), 2000);
    return () => window.clearTimeout(timer);
  }, [reloadNote]);

  return (
    <header className="editor-header">
      {showBack ? (
        <div className="editor-header-start">
          <a
            className="btn-back"
            href={
              backHref.startsWith('#')
                ? `${window.location.pathname}${window.location.search}${backHref}`
                : backHref
            }
          >
            <span style={{ userSelect: 'none' }}>{backLabel}</span>
          </a>
        </div>
      ) : null}
      <h1 className="editor-header-title">{title}</h1>
      <div className="editor-header-actions">
        {actions}
        {preserveAsset ? (
          <>
            {reloadNote ? (
              <span className="editor-header-note">{reloadNote}</span>
            ) : null}
            <Button
              variant="secondary"
              onClick={handleReload}
              disabled={loading}
              ariaLabel="Reload other assets (Alt+R)"
            >
              {loading ? 'Reloading…' : 'Reload assets'}
            </Button>
          </>
        ) : null}
        {onSave ? (
          <Button variant="primary" onClick={onSave}>
            {saveLabel}
          </Button>
        ) : null}
      </div>
    </header>
  );
}

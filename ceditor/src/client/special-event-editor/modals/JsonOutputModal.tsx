import { useState, useEffect } from 'react';
import { GameEvent } from '../../types/assets';
import { GenericModal } from '../../elements/GenericModal';
import { Button } from '../../elements/Button';
import { trimStrings } from '../../utils/jsonUtils';
import { syncGameEventFromEditorState, getEditorState } from '../seEditorState';

interface JsonOutputModalProps {
  isOpen: boolean;
  gameEvent: GameEvent | null;
  onCancel: () => void;
}

export function JsonOutputModal({
  isOpen,
  gameEvent,
  onCancel,
}: JsonOutputModalProps) {
  const [jsonOutput, setJsonOutput] = useState<string>('');
  const [copied, setCopied] = useState(false);

  useEffect(() => {
    if (isOpen && gameEvent) {
      // Create a copy to avoid mutating the original
      const gameEventCopy = JSON.parse(JSON.stringify(gameEvent));
      
      // Sync editor state to game event if needed
      const editorState = getEditorState();
      if (editorState.gameEventId === gameEvent.id) {
        syncGameEventFromEditorState(gameEventCopy, editorState);
      }
      
      // Prepare the game event for JSON output (same as saving)
      const trimmedGameEvent = trimStrings(gameEventCopy);
      
      // Convert to JSON with proper formatting
      const json = JSON.stringify(trimmedGameEvent, null, 2);
      setJsonOutput(json);
      setCopied(false);
    }
  }, [isOpen, gameEvent]);

  const handleCopy = async () => {
    try {
      await navigator.clipboard.writeText(jsonOutput);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    } catch (err) {
      console.error('Failed to copy:', err);
    }
  };

  if (!isOpen || !gameEvent) {
    return null;
  }

  return (
    <GenericModal
      title="JSON Output"
      onCancel={onCancel}
      onConfirm={onCancel}
      maxWidth="90vw"
      disableCancel={true}
      body={() => (
        <div>
          <div
            style={{
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              marginBottom: '10px',
            }}
          >
            <div style={{ color: '#d4d4d4', fontSize: '14px' }}>
              This is the JSON that would be saved for this game event:
            </div>
            <Button
              variant="secondary"
              onClick={handleCopy}
              style={{ minWidth: '100px' }}
            >
              {copied ? 'Copied!' : 'Copy JSON'}
            </Button>
          </div>
          <textarea
            readOnly
            value={jsonOutput}
            style={{
              width: '100%',
              minHeight: '400px',
              maxHeight: '70vh',
              padding: '12px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              fontFamily: 'monospace',
              fontSize: '13px',
              resize: 'vertical',
              whiteSpace: 'pre',
              overflowWrap: 'normal',
              overflowX: 'auto',
            }}
          />
        </div>
      )}
    />
  );
}

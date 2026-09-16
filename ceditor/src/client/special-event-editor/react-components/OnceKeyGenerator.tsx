import { useState } from 'react';
import { Button } from '../../elements/Button';

export function generateOnceKey(): string {
  const chars =
    'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
  const bytes = new Uint8Array(10);
  crypto.getRandomValues(bytes);
  return Array.from(bytes, (b) => chars[b % chars.length]).join('');
}

export function OnceKeyGenerator() {
  const [onceKey, setOnceKey] = useState('');

  const handleGenerateOnceKey = async () => {
    const generated = generateOnceKey();
    setOnceKey(generated);
    try {
      await navigator.clipboard.writeText(generated);
    } catch {
      // Selection in the shown field is enough to paste manually.
    }
  };

  return (
    <div
      className="once-key-generator"
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: '10px',
        marginBottom: '20px',
      }}
    >
      <Button
        variant="small"
        type="button"
        onClick={() => void handleGenerateOnceKey()}
      >
        Gen Random ID
      </Button>
      {onceKey && (
        <input
          readOnly
          value={onceKey}
          onFocus={(e) => e.currentTarget.select()}
          style={{
            width: '140px',
            padding: '6px 8px',
            backgroundColor: '#1e1e1e',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            color: '#d4d4d4',
            fontFamily: 'monospace',
            fontSize: '13px',
          }}
        />
      )}
    </div>
  );
}

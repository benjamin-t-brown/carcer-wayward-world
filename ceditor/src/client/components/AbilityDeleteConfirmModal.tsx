import type { ReactNode } from 'react';
import { Button } from '../elements/Button';
import { AbilityDeleteImpact } from '../types/assets';

interface AbilityDeleteConfirmModalProps {
  isOpen: boolean;
  title: string;
  description: ReactNode;
  impacts: AbilityDeleteImpact[];
  confirmLabel: string;
  onConfirm: () => void;
  onCancel: () => void;
}

function impactKindLabel(kind: AbilityDeleteImpact['kind']): string {
  return kind === 'item' ? 'Item' : 'Status effect';
}

export function AbilityDeleteConfirmModal({
  isOpen,
  title,
  description,
  impacts,
  confirmLabel,
  onConfirm,
  onCancel,
}: AbilityDeleteConfirmModalProps) {
  if (!isOpen) {
    return null;
  }

  return (
    <div
      className="generic-modal"
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: 'rgba(0, 0, 0, 0.7)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 1000,
      }}
      onClick={onCancel}
    >
      <div
        className="ability-delete-confirm-modal"
        style={{
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '8px',
          padding: '24px 28px',
          maxWidth: '560px',
          width: '92%',
          maxHeight: '85vh',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
          boxSizing: 'border-box',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <h2 style={{ color: '#f48771', margin: '0 0 12px', fontSize: '18px', flexShrink: 0 }}>
          {title}
        </h2>
        <div
          style={{
            color: '#d4d4d4',
            margin: '0 0 12px',
            lineHeight: 1.45,
            flexShrink: 0,
          }}
        >
          {description}
        </div>
        <div
          className="ability-delete-confirm-modal-impacts"
          style={{
            flex: '1 1 auto',
            minHeight: 0,
            maxHeight: 'min(50vh, 420px)',
            overflowY: 'auto',
            marginBottom: '16px',
            paddingRight: '8px',
            borderTop: '1px solid #3a3a3a',
            borderBottom: '1px solid #3a3a3a',
          }}
        >
          <ul
            style={{
              margin: '12px 0',
              padding: '0 0 0 20px',
              color: '#d4d4d4',
            }}
          >
            {impacts.map((impact) => (
              <li key={`${impact.kind}-${impact.name}`} style={{ marginBottom: '12px' }}>
                <div style={{ fontWeight: 600, color: '#e0e0e0' }}>
                  <span style={{ color: '#858585', fontWeight: 500, marginRight: '6px' }}>
                    {impactKindLabel(impact.kind)}
                  </span>
                  {impact.label}{' '}
                  <span style={{ fontWeight: 400, color: '#858585' }}>
                    ({impact.name})
                  </span>
                </div>
                <ul style={{ margin: '6px 0 0', paddingLeft: '18px' }}>
                  {impact.lines.map((line, lineIndex) => (
                    <li
                      key={`${impact.kind}-${impact.name}-${lineIndex}`}
                      style={{
                        marginBottom: '4px',
                        color: '#b0b0b0',
                        fontSize: '13px',
                      }}
                    >
                      {line}
                    </li>
                  ))}
                </ul>
              </li>
            ))}
          </ul>
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
            flexShrink: 0,
          }}
        >
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
          <Button variant="primary" className="btn-danger" onClick={onConfirm}>
            {confirmLabel}
          </Button>
        </div>
      </div>
    </div>
  );
}

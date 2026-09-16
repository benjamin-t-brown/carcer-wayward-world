import { useEffect, useMemo, useRef } from 'react';
import { GenericModal } from '../../elements/GenericModal';
import { CANVAS_CONTAINER_ID } from '../react-components/MapCanvasSE';
import { centerPanzoomOnNode } from '../seEditorState';
import { searchEditorNodes } from '../searchNodes';

interface SearchNodesModalProps {
  isOpen: boolean;
  query: string;
  onQueryChange: (query: string) => void;
  onClose: () => void;
}

export function SearchNodesModal({
  isOpen,
  query,
  onQueryChange,
  onClose,
}: SearchNodesModalProps) {
  const inputRef = useRef<HTMLInputElement>(null);
  const hits = useMemo(
    () => (isOpen ? searchEditorNodes(query) : []),
    [isOpen, query],
  );

  useEffect(() => {
    if (isOpen) {
      inputRef.current?.focus();
    }
  }, [isOpen]);

  const handleSelect = (nodeId: string) => {
    onClose();
    const canvas = document.getElementById(
      `${CANVAS_CONTAINER_ID}-canvas`,
    ) as HTMLCanvasElement | null;
    if (canvas) {
      centerPanzoomOnNode(canvas, nodeId, true);
    }
  };

  if (!isOpen) {
    return null;
  }

  return (
    <GenericModal
      title="Search Nodes"
      onCancel={onClose}
      onConfirm={onClose}
      maxWidth="720px"
      disableCancel
      fillBody
      body={() => (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            minHeight: 0,
            height: '100%',
            gap: '12px',
          }}
        >
          <input
            ref={inputRef}
            type="text"
            value={query}
            onChange={(event) => onQueryChange(event.target.value)}
            onKeyDown={(event) => {
              if (event.key === 'Enter' && hits[0]) {
                event.preventDefault();
                handleSelect(hits[0].nodeId);
              }
            }}
            placeholder="Exact search, case ignored…"
            style={{
              width: '100%',
              padding: '8px 10px',
              border: '1px solid #3e3e42',
              backgroundColor: '#1e1e1e',
              color: '#ffffff',
              fontSize: '14px',
              borderRadius: '4px',
              flexShrink: 0,
            }}
          />
          <div style={{ color: '#858585', fontSize: '12px', flexShrink: 0 }}>
            {!query.trim()
              ? "Type to search this event's nodes."
              : hits.length === 1
                ? '1 node'
                : `${hits.length} nodes`}
          </div>
          <div
            style={{
              flex: 1,
              minHeight: 0,
              overflowY: 'auto',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              backgroundColor: '#1e1e1e',
            }}
          >
            {query.trim() && hits.length === 0 ? (
              <div style={{ padding: '12px', color: '#858585' }}>
                No nodes match.
              </div>
            ) : null}
            {hits.map((hit) => (
              <button
                key={`${hit.nodeId}:${hit.field}:${hit.snippetMatch}`}
                type="button"
                onClick={() => handleSelect(hit.nodeId)}
                style={{
                  display: 'block',
                  width: '100%',
                  textAlign: 'left',
                  padding: '10px 12px',
                  background: 'transparent',
                  border: 'none',
                  borderBottom: '1px solid #3e3e42',
                  color: '#d4d4d4',
                  cursor: 'pointer',
                }}
                onMouseEnter={(event) => {
                  event.currentTarget.style.backgroundColor = '#3e3e42';
                }}
                onMouseLeave={(event) => {
                  event.currentTarget.style.backgroundColor = 'transparent';
                }}
              >
                <div
                  style={{
                    display: 'flex',
                    gap: '8px',
                    alignItems: 'baseline',
                    marginBottom: '4px',
                  }}
                >
                  <span
                    style={{
                      fontSize: '11px',
                      color: '#4ec9b0',
                      background: '#252526',
                      padding: '1px 6px',
                      borderRadius: '3px',
                    }}
                  >
                    {hit.nodeType}
                  </span>
                  <span
                    style={{
                      color: '#9cdcfe',
                      textDecoration: 'underline',
                      fontFamily: 'consolas, monospace',
                      fontSize: '13px',
                    }}
                  >
                    {hit.nodeId}
                  </span>
                  <span style={{ color: '#858585', fontSize: '11px' }}>
                    {hit.field}
                  </span>
                </div>
                <div
                  style={{
                    fontFamily: 'consolas, monospace',
                    fontSize: '12px',
                    color: '#cccccc',
                    whiteSpace: 'nowrap',
                    overflow: 'hidden',
                    textOverflow: 'ellipsis',
                  }}
                >
                  {hit.snippetBefore}
                  <span
                    style={{
                      backgroundColor: '#4ec9b0',
                      color: '#1e1e1e',
                      borderRadius: '2px',
                    }}
                  >
                    {hit.snippetMatch}
                  </span>
                  {hit.snippetAfter}
                </div>
              </button>
            ))}
          </div>
        </div>
      )}
    />
  );
}

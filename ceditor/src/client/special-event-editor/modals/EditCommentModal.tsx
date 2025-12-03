import { useState, useEffect } from 'react';
import { GameEvent } from '../../types/assets';
import { EditorNodeComment } from '../cmpts/CommentNodeComponent';
import { GenericModal } from '../../elements/GenericModal';

interface EditCommentModalProps {
  isOpen: boolean;
  node: EditorNodeComment | undefined;
  gameEvent: GameEvent;
  onCancel: () => void;
  ctx: CanvasRenderingContext2D;
}

export function EditCommentModal({
  isOpen,
  node,
  gameEvent,
  onCancel,
  ctx,
}: EditCommentModalProps) {
  const [text, setText] = useState<string>('');

  useEffect(() => {
    if (node) {
      const seNode = node.toSENode();
      setText(seNode.comment || '');
    }
  }, [node]);

  if (!isOpen || !node) {
    return null;
  }

  const handleEditNodeConfirm = () => {
    node.comment = text;
    node.calculateHeight(ctx);
    onCancel();
  };

  return (
    <GenericModal
      onCancel={onCancel}
      onConfirm={handleEditNodeConfirm}
      title="Edit Comment"
      body={() => (
        <textarea
          style={{
            width: '100%',
            minHeight: '200px',
            padding: '8px',
            backgroundColor: 'rgb(30, 30, 30)',
            border: '1px solid rgb(62, 62, 66)',
            borderRadius: '4px',
            color: 'rgb(212, 212, 212)',
            fontFamily: 'monospace',
            fontSize: '14px',
            resize: 'vertical',
          }}
          value={text}
          onChange={(e) => setText(e.target.value)}
        />
      )}
    ></GenericModal>
  );
}

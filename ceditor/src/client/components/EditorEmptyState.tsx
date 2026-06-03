interface EditorEmptyStateProps {
  message: string;
}

export function EditorEmptyState({ message }: EditorEmptyStateProps) {
  return (
    <div className="editor-empty-state">
      <p>{message}</p>
    </div>
  );
}

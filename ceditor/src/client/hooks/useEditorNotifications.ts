import { useCallback, useRef, useState } from 'react';

export interface EditorNotification {
  id: number;
  message: string;
  type: 'success' | 'error';
  duration?: number;
}

export function useEditorNotifications() {
  const [notifications, setNotifications] = useState<EditorNotification[]>([]);
  const nextIdRef = useRef(0);

  const showNotification = useCallback(
    (message: string, type: EditorNotification['type'], duration?: number) => {
      const id = nextIdRef.current++;
      setNotifications((current) => [
        ...current,
        { id, message, type, duration },
      ]);
    },
    [],
  );

  const removeNotification = useCallback((id: number) => {
    setNotifications((current) =>
      current.filter((notification) => notification.id !== id),
    );
  }, []);

  return { notifications, showNotification, removeNotification };
}

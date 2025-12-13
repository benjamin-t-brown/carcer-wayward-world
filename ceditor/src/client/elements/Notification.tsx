import React, { useEffect, useState, useRef } from 'react';

interface NotificationProps {
  message: string;
  type: 'success' | 'error';
  duration?: number;
  onClose?: () => void;
}

export const Notification: React.FC<NotificationProps> = ({
  message,
  type,
  duration = 1000,
  onClose,
}) => {
  const [show, setShow] = useState(false);
  const timerRef = useRef<NodeJS.Timeout | null>(null);

  const handleClose = () => {
    if (timerRef.current) {
      clearTimeout(timerRef.current);
      timerRef.current = null;
    }
    setShow(false);
    setTimeout(() => {
      onClose?.();
    }, 300); // Wait for fade-out animation
  };

  useEffect(() => {
    // Trigger show animation
    setTimeout(() => setShow(true), 10);

    // Auto-hide after duration
    timerRef.current = setTimeout(() => {
      setShow(false);
      setTimeout(() => {
        onClose?.();
      }, 300); // Wait for fade-out animation
    }, duration);

    return () => {
      if (timerRef.current) {
        clearTimeout(timerRef.current);
      }
    };
  }, [duration, onClose]);

  return (
    <div className={`notification ${type} ${show ? 'show' : ''}`}>
      <span style={{ flex: 1, marginRight: '10px' }}>{message}</span>
      <button
        onClick={handleClose}
        style={{
          background: 'none',
          border: 'none',
          color: 'inherit',
          cursor: 'pointer',
          fontSize: '18px',
          fontWeight: 'bold',
          padding: '0',
          marginLeft: 'auto',
          lineHeight: '1',
          opacity: 0.7,
          transition: 'opacity 0.2s',
          flexShrink: 0,
        }}
        onMouseEnter={(e) => {
          e.currentTarget.style.opacity = '1';
        }}
        onMouseLeave={(e) => {
          e.currentTarget.style.opacity = '0.7';
        }}
        aria-label="Close notification"
      >
        ×
      </button>
    </div>
  );
};


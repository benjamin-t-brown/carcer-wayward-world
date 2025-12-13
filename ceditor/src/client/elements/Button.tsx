import React from 'react';

type ButtonVariant = 'primary' | 'secondary' | 'danger' | 'back' | 'small';

interface ButtonProps {
  children: React.ReactNode;
  onClick?: () => void;
  variant?: ButtonVariant;
  type?: 'button' | 'submit' | 'reset';
  disabled?: boolean;
  className?: string;
}

export const Button: React.FC<ButtonProps> = ({
  children,
  onClick,
  variant = 'primary',
  type = 'button',
  disabled = false,
  className = '',
}) => {
  const getButtonClass = () => {
    switch (variant) {
      case 'primary':
        return 'btn-primary';
      case 'secondary':
        return 'btn-secondary';
      case 'danger':
        return 'btn-danger';
      case 'back':
        return 'btn-back';
      case 'small':
        return 'btn-small';
      default:
        return 'btn-primary';
    }
  };

  const classes = `${getButtonClass()} ${className}`.trim();

  return (
    <button
      type={type}
      className={classes}
      onClick={onClick}
      disabled={disabled}
    >
      <span style={{ userSelect: 'none' }}>{children}</span>
    </button>
  );
};

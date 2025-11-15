import React from 'react';

interface CardProps {
  id?: string;
  children: React.ReactNode;
  onClick?: () => void;
  className?: string;
  variant?: 'asset' | 'item';
}

export const Card: React.FC<CardProps> = ({
  id,
  children,
  onClick,
  className = '',
  variant = 'asset',
}) => {
  const baseClass = variant === 'asset' ? 'asset-card' : 'item-card';
  const classes = `${baseClass} ${className}`.trim();

  return (
    <div
      id={id}
      className={classes}
      onClick={onClick}
      style={{ cursor: onClick ? 'pointer' : 'default' }}
    >
      {children}
    </div>
  );
};


import React from 'react';
import { Card } from './Card';

export interface SelectableListCardProps {
  id?: string;
  selected?: boolean;
  onClick?: () => void;
  /** Primary heading; enables the standard header + body layout when set. */
  title?: string;
  /** Secondary identifier shown under the title (e.g. internal name / id). */
  subtitle?: string;
  /** Clone, delete, etc. Rendered in the header; clicks do not select the card. */
  actions?: React.ReactNode;
  /** Extra content in the card body (standard layout) or full card content (custom layout). */
  children?: React.ReactNode;
}

/**
 * Clickable sidebar list card used across template editors.
 * Pass `title` for the common label + subtitle + actions layout; omit `title` for fully custom content.
 */
export function SelectableListCard({
  id,
  selected = false,
  onClick,
  title,
  subtitle,
  actions,
  children,
}: SelectableListCardProps) {
  const useStandardLayout = title !== undefined;

  return (
    <Card
      id={id}
      variant="item"
      className={selected ? 'editing' : ''}
      onClick={onClick}
    >
      {useStandardLayout ? (
        <>
          <div className="item-card-header">
            <h3>{title}</h3>
            {actions && (
              <div
                className="item-card-actions"
                onClick={(e) => e.stopPropagation()}
              >
                {actions}
              </div>
            )}
          </div>
          <div className="item-card-body">
            {subtitle !== undefined && subtitle !== '' && (
              <div className="item-info">
                <span className="item-name">{subtitle}</span>
              </div>
            )}
            {children}
          </div>
        </>
      ) : (
        children
      )}
    </Card>
  );
}

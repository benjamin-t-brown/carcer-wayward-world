import React from 'react';
import { InputTinge, tingeClassName } from './inputTinge';

interface TextInputProps {
  id?: string;
  name?: string;
  label?: string;
  value: string;
  onChange: (value: string) => void;
  onKeyDown?: (e: React.KeyboardEvent<HTMLInputElement>) => void;
  onBlur?: (e: React.FocusEvent<HTMLInputElement>) => void;
  placeholder?: string;
  required?: boolean;
  disabled?: boolean;
  inputClassName?: string;
  tinge?: InputTinge;
}

function isWideNameOrLabelField(name?: string, label?: string): boolean {
  if (name === 'name' || name === 'label') {
    return true;
  }
  if (!label) {
    return false;
  }
  const normalized = label.replace(/\s*\*$/, '').trim();
  return /^name(\s*\(id\))?$/i.test(normalized) || /^label$/i.test(normalized);
}

function mergeInputClassName(
  name?: string,
  label?: string,
  inputClassName?: string
): string | undefined {
  const classes = [
    isWideNameOrLabelField(name, label) ? 'wide-input' : '',
    inputClassName ?? '',
  ].filter(Boolean);
  return classes.length > 0 ? classes.join(' ') : undefined;
}

export const TextInput: React.FC<TextInputProps> = ({
  id,
  name,
  label,
  value,
  onChange,
  placeholder,
  required = false,
  disabled = false,
  onKeyDown,
  onBlur,
  inputClassName,
  tinge,
}) => {
  return (
    <div
      className={['form-group', tingeClassName(tinge)].filter(Boolean).join(' ')}
      style={{
      filter: disabled ? 'grayscale(100%) brightness(1.5)' : 'none',
    }}>
      {label && (
        <label htmlFor={id}>
          {label}
          {required && ' *'}
        </label>
      )}
      <input
        type="text"
        id={id}
        name={name}
        value={value}
        onChange={(e) => onChange(e.target.value)}
        onKeyDown={onKeyDown}
        onBlur={onBlur}
        placeholder={placeholder}
        required={required}
        disabled={disabled}
        className={mergeInputClassName(name, label, inputClassName)}
      />
    </div>
  );
};


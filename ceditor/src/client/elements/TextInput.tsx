import React from 'react';

interface TextInputProps {
  id?: string;
  name?: string;
  label?: string;
  value: string;
  onChange: (value: string) => void;
  onKeyDown?: (e: React.KeyboardEvent<HTMLInputElement>) => void;
  placeholder?: string;
  required?: boolean;
  disabled?: boolean;
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
}) => {
  return (
    <div className="form-group" style={{
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
        placeholder={placeholder}
        required={required}
        disabled={disabled}
      />
    </div>
  );
};


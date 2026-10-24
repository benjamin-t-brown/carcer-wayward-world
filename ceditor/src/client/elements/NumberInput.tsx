import React from 'react';
import { InputTinge, tingeClassName } from './inputTinge';

interface NumberInputProps {
  id?: string;
  name?: string;
  label?: string;
  value: number;
  onChange: (value: number) => void;
  placeholder?: string;
  min?: number;
  max?: number;
  required?: boolean;
  disabled?: boolean;
  tinge?: InputTinge;
}

export const NumberInput: React.FC<NumberInputProps> = ({
  id,
  name,
  label,
  value,
  onChange,
  placeholder,
  min,
  max,
  required = false,
  disabled = false,
  tinge,
}) => {
  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const numValue = parseFloat(e.target.value);
    if (!isNaN(numValue)) {
      onChange(numValue);
    } else if (e.target.value === '') {
      onChange(0);
    }
  };

  return (
    <div
      className={['form-group', tingeClassName(tinge)].filter(Boolean).join(' ')}
      style={{
        filter: disabled
          ? 'grayscale(100%) brightness(0.5) sepia(100%)'
          : 'none',
      }}
    >
      {label && (
        <label htmlFor={id}>
          {label}
          {required && ' *'}
        </label>
      )}
      <input
        type="number"
        id={id}
        name={name}
        value={value}
        onChange={handleChange}
        placeholder={placeholder}
        min={min}
        max={max}
        required={required}
        disabled={disabled}
        style={{
          maxWidth: '125px',
        }}
      />
    </div>
  );
};

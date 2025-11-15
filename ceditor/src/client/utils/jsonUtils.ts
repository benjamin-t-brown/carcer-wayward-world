/**
 * Recursively trims all string values in a JSON object or array.
 * Works with nested objects, arrays, and primitive values.
 * 
 * @param value - The value to trim (can be any JSON-serializable value)
 * @returns A new object/array/value with all strings trimmed
 */
export function trimStrings<T>(value: T): T {
  // Handle null or undefined
  if (value === null || value === undefined) {
    return value;
  }

  // Handle strings - trim them
  if (typeof value === 'string') {
    return value.trim() as T;
  }

  // Handle arrays - recursively trim each element
  if (Array.isArray(value)) {
    return value.map((item) => trimStrings(item)) as T;
  }

  // Handle objects - recursively trim each property
  if (typeof value === 'object') {
    const trimmed: Record<string, any> = {};
    for (const [key, val] of Object.entries(value)) {
      trimmed[key] = trimStrings(val);
    }
    return trimmed as T;
  }

  // For primitives (number, boolean, etc.), return as-is
  return value;
}


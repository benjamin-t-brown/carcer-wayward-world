import { CarcerMapTemplate, MAP_TYPES } from '../types/assets';

export const validateMaps = (
  maps: CarcerMapTemplate[],
): { isValid: boolean; error?: string } => {
  const errors: string[] = [];
  const nameCounts = new Map<string, number>();
  const mapsWithMissingFields: string[] = [];

  maps.forEach((map, index) => {
    const missingFields: string[] = [];

    // Check required string fields
    if (!map.name || map.name.trim() === '') {
      missingFields.push('name');
    }
    if (!map.label || map.label.trim() === '') {
      missingFields.push('label');
    }
    if (!map.type || !MAP_TYPES.includes(map.type)) {
      missingFields.push('type');
    }

    // Check required number fields
    if (map.width === undefined || map.width === null || map.width <= 0) {
      missingFields.push('width');
    }
    if (map.height === undefined || map.height === null || map.height <= 0) {
      missingFields.push('height');
    }

    if (missingFields.length > 0) {
      const mapIdentifier = map.name || `Map at index ${index}`;
      mapsWithMissingFields.push(
        `${mapIdentifier}: missing ${missingFields.join(', ')}`,
      );
    }

    // Track names for duplicate checking
    if (map.name && map.name.trim()) {
      const count = nameCounts.get(map.name) || 0;
      nameCounts.set(map.name, count + 1);
    }
  });

  // Check for duplicate names
  const duplicateNames: string[] = [];
  nameCounts.forEach((count, name) => {
    if (count > 1) {
      duplicateNames.push(name);
    }
  });

  if (duplicateNames.length > 0) {
    errors.push(`Duplicate map names found: ${duplicateNames.join(', ')}`);
  }

  if (mapsWithMissingFields.length > 0) {
    errors.push(
      `Maps with missing required fields:\n${mapsWithMissingFields.join('\n')}`,
    );
  }

  if (errors.length > 0) {
    return {
      isValid: false,
      error: errors.join('\n\n'),
    };
  }

  return { isValid: true };
};

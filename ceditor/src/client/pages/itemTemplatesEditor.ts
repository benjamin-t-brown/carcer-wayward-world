interface ItemTemplate {
  itemType: string;
  name: string;
  label: string;
  icon: string;
  description: string;
  weight: number;
  value: number;
  // Optional fields
  itemUsability?: string;
  itemUsabilityArgs?: {
    itemUsabilityType: string;
    intArgs?: number[];
    stringArgs?: string[];
  };
  statusEffects?: Array<{
    statusEffectType: string;
    name?: string;
    label?: string;
    description?: string;
  }>;
  weapon?: {
    dmgMin?: number;
    dmgMax?: number;
  };
}

const ITEM_TYPES = [
  'WEAPON_MELEE',
  'WEAPON_RANGED',
  'WEAPON_AMMO',
  'GARB',
  'PANTS',
  'GLOVES',
  'HAT',
  'SHOES',
  'NECKLACE',
  'POTION',
  'UTILITY',
];

const ITEM_USABILITY_TYPES = [
  'NOT_USABLE',
  'USABLE_EVERYWHERE',
  'USABLE_TOWN_ONLY',
  'USABLE_COMBAT_ONLY',
  'USABLE_OUTSIDE_ONLY',
  'USABLE_TOWN_AND_COMBAT',
];

const ITEM_USABILITY_ARG_TYPES = [
  'ITEM_USE_NOT_USABLE',
  'ITEM_USE_CAST_SPELL',
];

const STATUS_EFFECT_TYPES = [
  'MODIFY_RESISTANCE_FIRE_PLUS_1',
];

let items: ItemTemplate[] = [];
let currentEditingIndex: number | null = null;

async function loadItems(): Promise<ItemTemplate[]> {
  const response = await fetch('/api/assets/itemTemplates');
  if (!response.ok) {
    throw new Error('Failed to load items');
  }
  return response.json();
}

async function saveItems() {
  try {
    const response = await fetch('/api/assets/itemTemplates', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(items),
    });

    if (!response.ok) {
      throw new Error('Failed to save items');
    }

    showNotification('Items saved successfully!', 'success');
  } catch (error) {
    showNotification(`Error saving: ${error instanceof Error ? error.message : 'Unknown error'}`, 'error');
  }
}

function showNotification(message: string, type: 'success' | 'error') {
  const notification = document.createElement('div');
  notification.className = `notification ${type}`;
  notification.textContent = message;
  document.body.appendChild(notification);

  setTimeout(() => {
    notification.classList.add('show');
  }, 10);

  setTimeout(() => {
    notification.classList.remove('show');
    setTimeout(() => notification.remove(), 300);
  }, 3000);
}

function renderItemForm(item: ItemTemplate | null, index: number | null) {
  const form = document.getElementById('item-form');
  if (!form) return;

  if (item) {
    // Required fields
    (form.querySelector('#item-type') as HTMLSelectElement).value = item.itemType;
    (form.querySelector('#item-name') as HTMLInputElement).value = item.name;
    (form.querySelector('#item-label') as HTMLInputElement).value = item.label;
    (form.querySelector('#item-icon') as HTMLInputElement).value = item.icon;
    (form.querySelector('#item-description') as HTMLTextAreaElement).value = item.description;
    (form.querySelector('#item-weight') as HTMLInputElement).value = item.weight.toString();
    (form.querySelector('#item-value') as HTMLInputElement).value = item.value.toString();

    // Optional: Usability
    (form.querySelector('#item-usability') as HTMLSelectElement).value = item.itemUsability || 'NOT_USABLE';
    
    // Optional: Usability Args
    const usabilityType = item.itemUsabilityArgs?.itemUsabilityType || 'ITEM_USE_NOT_USABLE';
    (form.querySelector('#item-usability-type') as HTMLSelectElement).value = usabilityType;
    
    const intArgs = item.itemUsabilityArgs?.intArgs || [];
    (form.querySelector('#item-usability-int-args') as HTMLInputElement).value = intArgs.join(',');
    
    const stringArgs = item.itemUsabilityArgs?.stringArgs || [];
    (form.querySelector('#item-usability-string-args') as HTMLInputElement).value = stringArgs.join(',');

    // Optional: Weapon
    if (item.weapon) {
      (form.querySelector('#weapon-dmg-min') as HTMLInputElement).value = (item.weapon.dmgMin || '').toString();
      (form.querySelector('#weapon-dmg-max') as HTMLInputElement).value = (item.weapon.dmgMax || '').toString();
    } else {
      (form.querySelector('#weapon-dmg-min') as HTMLInputElement).value = '';
      (form.querySelector('#weapon-dmg-max') as HTMLInputElement).value = '';
    }

    // Optional: Status Effects
    renderStatusEffects(item.statusEffects || []);

    currentEditingIndex = index;
  } else {
    form.querySelector('form')?.reset();
    renderStatusEffects([]);
    currentEditingIndex = null;
  }
}

function renderStatusEffects(statusEffects: Array<{ statusEffectType: string; name?: string; label?: string; description?: string }>) {
  const container = document.getElementById('status-effects-list');
  if (!container) return;

  container.innerHTML = statusEffects
    .map(
      (effect, index) => `
    <div class="status-effect-item" data-index="${index}">
      <div class="status-effect-header">
        <select class="status-effect-type" data-index="${index}">
          ${STATUS_EFFECT_TYPES.map((type) => `<option value="${type}" ${effect.statusEffectType === type ? 'selected' : ''}>${type}</option>`).join('')}
        </select>
        <button type="button" class="btn-small btn-danger" onclick="window.removeStatusEffect(${index})">Remove</button>
      </div>
      <div class="form-group">
        <label>Name</label>
        <input type="text" class="status-effect-name" data-index="${index}" value="${effect.name || ''}" placeholder="Status effect name" />
      </div>
      <div class="form-group">
        <label>Label</label>
        <input type="text" class="status-effect-label" data-index="${index}" value="${effect.label || ''}" placeholder="Display label" />
      </div>
      <div class="form-group">
        <label>Description</label>
        <textarea class="status-effect-description" data-index="${index}" rows="2" placeholder="Description">${effect.description || ''}</textarea>
      </div>
    </div>
  `
    )
    .join('');

  // Make remove function available globally
  (window as any).removeStatusEffect = (index: number) => {
    const item = items[currentEditingIndex!];
    if (item.statusEffects) {
      item.statusEffects.splice(index, 1);
      renderStatusEffects(item.statusEffects);
    }
  };
}

function createNewItem() {
  const newItem: ItemTemplate = {
    itemType: 'UTILITY',
    name: '',
    label: '',
    icon: '',
    description: '',
    weight: 0,
    value: 0,
    itemUsability: 'NOT_USABLE',
  };
  items.push(newItem);
  renderItemList();
  renderItemForm(newItem, items.length - 1);
  document.getElementById('item-form')?.scrollIntoView({ behavior: 'smooth' });
}

function deleteItem(index: number) {
  if (confirm('Are you sure you want to delete this item?')) {
    items.splice(index, 1);
    renderItemList();
    if (currentEditingIndex === index) {
      renderItemForm(null, null);
    } else if (currentEditingIndex !== null && currentEditingIndex > index) {
      currentEditingIndex--;
    }
  }
}

function saveItemFromForm() {
  const form = document.getElementById('item-form')?.querySelector('form');
  if (!form) return;

  const formData = new FormData(form as HTMLFormElement);
  const item: ItemTemplate = {
    itemType: formData.get('itemType') as string,
    name: formData.get('name') as string,
    label: formData.get('label') as string,
    icon: formData.get('icon') as string,
    description: formData.get('description') as string,
    weight: parseInt(formData.get('weight') as string) || 0,
    value: parseInt(formData.get('value') as string) || 0,
  };

  if (!item.name) {
    showNotification('Item name is required', 'error');
    return;
  }

  // Optional: Usability
  const usability = formData.get('itemUsability') as string;
  if (usability && usability !== 'NOT_USABLE') {
    item.itemUsability = usability;
    
    const usabilityType = formData.get('itemUsabilityType') as string;
    const intArgsStr = (formData.get('itemUsabilityIntArgs') as string)?.trim();
    const stringArgsStr = (formData.get('itemUsabilityStringArgs') as string)?.trim();
    
    if (usabilityType && usabilityType !== 'ITEM_USE_NOT_USABLE') {
      const args: any = {
        itemUsabilityType: usabilityType,
      };
      if (intArgsStr) {
        const intArgs = intArgsStr.split(',').map(s => parseInt(s.trim())).filter(n => !isNaN(n));
        if (intArgs.length > 0) args.intArgs = intArgs;
      }
      if (stringArgsStr) {
        const stringArgs = stringArgsStr.split(',').map(s => s.trim()).filter(s => s.length > 0);
        if (stringArgs.length > 0) args.stringArgs = stringArgs;
      }
      if (Object.keys(args).length > 1 || args.itemUsabilityType !== 'ITEM_USE_NOT_USABLE') {
        item.itemUsabilityArgs = args;
      }
    } else if (intArgsStr || stringArgsStr) {
      // If usability type is not set but args are, still create the args object
      const args: any = {
        itemUsabilityType: 'ITEM_USE_NOT_USABLE',
      };
      if (intArgsStr) {
        const intArgs = intArgsStr.split(',').map(s => parseInt(s.trim())).filter(n => !isNaN(n));
        if (intArgs.length > 0) args.intArgs = intArgs;
      }
      if (stringArgsStr) {
        const stringArgs = stringArgsStr.split(',').map(s => s.trim()).filter(s => s.length > 0);
        if (stringArgs.length > 0) args.stringArgs = stringArgs;
      }
      if (Object.keys(args).length > 1) {
        item.itemUsabilityArgs = args;
      }
    }
  }

  // Optional: Weapon damage
  const dmgMin = (formData.get('weaponDmgMin') as string)?.trim();
  const dmgMax = (formData.get('weaponDmgMax') as string)?.trim();
  if (dmgMin || dmgMax) {
    const weapon: any = {};
    if (dmgMin) {
      const min = parseInt(dmgMin);
      if (!isNaN(min)) weapon.dmgMin = min;
    }
    if (dmgMax) {
      const max = parseInt(dmgMax);
      if (!isNaN(max)) weapon.dmgMax = max;
    }
    if (Object.keys(weapon).length > 0) {
      item.weapon = weapon;
    }
  }

  // Optional: Status Effects
  const statusEffects: Array<{ statusEffectType: string; name?: string; label?: string; description?: string }> = [];
  document.querySelectorAll('.status-effect-item').forEach((element) => {
    const index = (element as HTMLElement).dataset.index;
    const typeSelect = element.querySelector('.status-effect-type') as HTMLSelectElement;
    const nameInput = element.querySelector('.status-effect-name') as HTMLInputElement;
    const labelInput = element.querySelector('.status-effect-label') as HTMLInputElement;
    const descTextarea = element.querySelector('.status-effect-description') as HTMLTextAreaElement;

    if (typeSelect) {
      const effect: any = {
        statusEffectType: typeSelect.value,
      };
      if (nameInput?.value) effect.name = nameInput.value;
      if (labelInput?.value) effect.label = labelInput.value;
      if (descTextarea?.value) effect.description = descTextarea.value;
      statusEffects.push(effect);
    }
  });
  if (statusEffects.length > 0) {
    item.statusEffects = statusEffects;
  }

  if (currentEditingIndex !== null) {
    items[currentEditingIndex] = item;
  } else {
    items.push(item);
  }

  renderItemList();
  renderItemForm(null, null);
  showNotification('Item saved to list (click "Save All" to update file)', 'success');
}

function renderItemList() {
  const list = document.getElementById('item-list');
  if (!list) return;

  const searchTerm = (document.getElementById('search') as HTMLInputElement)?.value.toLowerCase() || '';

  const filteredItems = items.filter(
    (item) =>
      item.name.toLowerCase().includes(searchTerm) ||
      item.label.toLowerCase().includes(searchTerm) ||
      item.description.toLowerCase().includes(searchTerm)
  );

  if (filteredItems.length === 0) {
    list.innerHTML = '<div class="empty-state">No items found</div>';
    return;
  }

  list.innerHTML = filteredItems
    .map((item, index) => {
      const actualIndex = items.indexOf(item);
      return `
      <div class="item-card ${currentEditingIndex === actualIndex ? 'editing' : ''}" onclick="window.editItem(${actualIndex})">
        <div class="item-card-header">
          <h3>${item.label || item.name || 'Unnamed Item'}</h3>
          <div class="item-card-actions" onclick="event.stopPropagation()">
            <button class="btn-small" onclick="window.cloneItem(${actualIndex})">Clone</button>
            <button class="btn-small btn-danger" onclick="window.deleteItem(${actualIndex})">Delete</button>
          </div>
        </div>
        <div class="item-card-body">
          <div class="item-info">
            <span class="item-type">${item.itemType}</span>
            <span class="item-name">${item.name}</span>
          </div>
          <p class="item-description">${item.description || 'No description'}</p>
          <div class="item-stats">
            <span>Weight: ${item.weight}</span>
            <span>Value: ${item.value}</span>
          </div>
        </div>
      </div>
    `;
    })
    .join('');

  // Make functions available globally for onclick handlers
  (window as any).editItem = (index: number) => {
    renderItemForm(items[index], index);
    document.getElementById('item-form')?.scrollIntoView({ behavior: 'smooth' });
  };
  (window as any).cloneItem = (index: number) => {
    const originalItem = items[index];
    const clonedItem: ItemTemplate = JSON.parse(JSON.stringify(originalItem));
    clonedItem.name = clonedItem.name + '_copy';
    items.push(clonedItem);
    renderItemList();
    renderItemForm(clonedItem, items.length - 1);
    document.getElementById('item-form')?.scrollIntoView({ behavior: 'smooth' });
    showNotification('Item cloned!', 'success');
  };
  (window as any).deleteItem = deleteItem;
}

export async function renderItemTemplatesEditor() {
  const app = document.getElementById('app');
  if (!app) return;

  app.innerHTML = `
    <div class="container">
      <div class="editor-header">
        <button class="btn-back" onclick="window.history.back()">← Back</button>
        <h1>Item Templates Editor</h1>
        <button class="btn-primary" id="save-all-btn">Save All</button>
      </div>

      <div class="editor-content">
        <div class="editor-sidebar">
          <div class="search-box">
            <input type="text" id="search" placeholder="Search items..." />
          </div>
          <div class="item-actions">
            <button class="btn-primary" id="new-item-btn">+ New Item</button>
          </div>
          <div id="item-list" class="item-list">
            <div class="loading">Loading items...</div>
          </div>
        </div>

        <div class="editor-main">
          <div id="item-form" class="item-form">
            <h2>${currentEditingIndex !== null ? 'Edit Item' : 'New Item'}</h2>
            <form id="item-form-element">
              <div class="form-group">
                <label for="item-type">Item Type *</label>
                <select id="item-type" name="itemType" required>
                  ${ITEM_TYPES.map((type) => `<option value="${type}">${type}</option>`).join('')}
                </select>
              </div>

              <div class="form-group">
                <label for="item-name">Name *</label>
                <input type="text" id="item-name" name="name" required placeholder="e.g., PotionHealing" />
              </div>

              <div class="form-group">
                <label for="item-label">Label *</label>
                <input type="text" id="item-label" name="label" required placeholder="e.g., Potion: Healing" />
              </div>

              <div class="form-group">
                <label for="item-icon">Icon *</label>
                <input type="text" id="item-icon" name="icon" required placeholder="e.g., potionGreen" />
              </div>

              <div class="form-group">
                <label for="item-description">Description *</label>
                <textarea id="item-description" name="description" required rows="3" placeholder="Item description"></textarea>
              </div>

              <div class="form-row">
                <div class="form-group">
                  <label for="item-weight">Weight *</label>
                  <input type="number" id="item-weight" name="weight" required min="0" />
                </div>

                <div class="form-group">
                  <label for="item-value">Value *</label>
                  <input type="number" id="item-value" name="value" required min="0" />
                </div>
              </div>

              <div class="form-section">
                <h3>Optional Properties</h3>

                <div class="form-group">
                  <label for="item-usability">Usability</label>
                  <select id="item-usability" name="itemUsability">
                    ${ITEM_USABILITY_TYPES.map((type) => `<option value="${type}">${type}</option>`).join('')}
                  </select>
                </div>

                <div class="form-group">
                  <label for="item-usability-type">Usability Type</label>
                  <select id="item-usability-type" name="itemUsabilityType">
                    ${ITEM_USABILITY_ARG_TYPES.map((type) => `<option value="${type}">${type}</option>`).join('')}
                  </select>
                </div>

                <div class="form-group">
                  <label for="item-usability-int-args">Usability Int Args (comma-separated)</label>
                  <input type="text" id="item-usability-int-args" name="itemUsabilityIntArgs" placeholder="e.g., 10, 20, 30" />
                </div>

                <div class="form-group">
                  <label for="item-usability-string-args">Usability String Args (comma-separated)</label>
                  <input type="text" id="item-usability-string-args" name="itemUsabilityStringArgs" placeholder="e.g., arg1, arg2" />
                </div>

                <div class="form-row">
                  <div class="form-group">
                    <label for="weapon-dmg-min">Weapon Damage Min</label>
                    <input type="number" id="weapon-dmg-min" name="weaponDmgMin" min="0" />
                  </div>

                  <div class="form-group">
                    <label for="weapon-dmg-max">Weapon Damage Max</label>
                    <input type="number" id="weapon-dmg-max" name="weaponDmgMax" min="0" />
                  </div>
                </div>

                <div class="form-group">
                  <label>Status Effects</label>
                  <div id="status-effects-list"></div>
                  <button type="button" class="btn-secondary" id="add-status-effect-btn" style="margin-top: 10px;">+ Add Status Effect</button>
                </div>
              </div>

              <div class="form-actions">
                <button type="button" class="btn-secondary" id="cancel-btn">Cancel</button>
                <button type="submit" class="btn-primary">Save Item</button>
              </div>
            </form>
          </div>
        </div>
      </div>
    </div>
  `;

  // Load items
  try {
    items = await loadItems();
    renderItemList();
  } catch (error) {
    const list = document.getElementById('item-list');
    if (list) {
      list.innerHTML = `<div class="error">Error loading items: ${error instanceof Error ? error.message : 'Unknown error'}</div>`;
    }
  }

  // Event listeners
  document.getElementById('search')?.addEventListener('input', renderItemList);
  document.getElementById('new-item-btn')?.addEventListener('click', createNewItem);
  document.getElementById('save-all-btn')?.addEventListener('click', saveItems);
  document.getElementById('cancel-btn')?.addEventListener('click', () => {
    renderItemForm(null, null);
  });
  document.getElementById('add-status-effect-btn')?.addEventListener('click', () => {
    // Get current form values to create a temporary item for status effects
    const form = document.getElementById('item-form-element') as HTMLFormElement;
    if (!form) return;
    
    // Create or get item for status effects
    let item: ItemTemplate;
    if (currentEditingIndex !== null) {
      item = items[currentEditingIndex];
    } else {
      // For new items, create a temporary structure
      item = {
        itemType: 'UTILITY',
        name: '',
        label: '',
        icon: '',
        description: '',
        weight: 0,
        value: 0,
        statusEffects: [],
      };
      items.push(item);
      currentEditingIndex = items.length - 1;
    }
    
    if (!item.statusEffects) {
      item.statusEffects = [];
    }
    item.statusEffects.push({
      statusEffectType: 'MODIFY_RESISTANCE_FIRE_PLUS_1',
    });
    renderStatusEffects(item.statusEffects);
  });
  document.getElementById('item-form-element')?.addEventListener('submit', (e) => {
    e.preventDefault();
    saveItemFromForm();
  });
}


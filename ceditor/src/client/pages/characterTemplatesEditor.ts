interface CharacterTemplate {
  type: string;
  name: string;
  label: string;
  spritesheet: string;
  spriteOffset: string;
  // Optional fields
  talk?: {
    talkName?: string;
    portraitName?: string;
  };
  combat?: {
    stats?: {
      str?: number;
      mnd?: number;
      con?: number;
      agi?: number;
      lck?: number;
    };
    hp?: number;
    mp?: number;
    behaviorType?: string;
    dropTable?: string;
  };
  combatTemplate?: {
    id?: string;
  };
  sound?: {
    deathSound?: string;
    weaponSound?: string;
  };
  statuses?: string[];
  vision?: {
    radius?: number;
  };
}

const CHARACTER_TYPES = [
  'TOWNSPERSON',
  'TOWNSPERSON_STATIC',
  'ENEMY',
  'ENEMY_STATIC',
];

let characters: CharacterTemplate[] = [];
let currentEditingIndex: number | null = null;

async function loadCharacters(): Promise<CharacterTemplate[]> {
  const response = await fetch('/api/assets/characterTemplates');
  if (!response.ok) {
    throw new Error('Failed to load characters');
  }
  return response.json();
}

async function saveCharacters() {
  try {
    const response = await fetch('/api/assets/characterTemplates', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(characters),
    });

    if (!response.ok) {
      throw new Error('Failed to save characters');
    }

    showNotification('Characters saved successfully!', 'success');
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

function renderCharacterForm(character: CharacterTemplate | null, index: number | null) {
  const form = document.getElementById('character-form');
  if (!form) return;

  if (character) {
    // Required fields
    (form.querySelector('#character-type') as HTMLSelectElement).value = character.type;
    (form.querySelector('#character-name') as HTMLInputElement).value = character.name;
    (form.querySelector('#character-label') as HTMLInputElement).value = character.label;
    (form.querySelector('#character-spritesheet') as HTMLInputElement).value = character.spritesheet;
    (form.querySelector('#character-sprite-offset') as HTMLInputElement).value = character.spriteOffset;

    // Optional: Talk
    if (character.talk) {
      (form.querySelector('#talk-name') as HTMLInputElement).value = character.talk.talkName || '';
      (form.querySelector('#talk-portrait') as HTMLInputElement).value = character.talk.portraitName || '';
    } else {
      (form.querySelector('#talk-name') as HTMLInputElement).value = '';
      (form.querySelector('#talk-portrait') as HTMLInputElement).value = '';
    }

    // Optional: Combat
    if (character.combat) {
      if (character.combat.stats) {
        (form.querySelector('#combat-str') as HTMLInputElement).value = (character.combat.stats.str || '').toString();
        (form.querySelector('#combat-mnd') as HTMLInputElement).value = (character.combat.stats.mnd || '').toString();
        (form.querySelector('#combat-con') as HTMLInputElement).value = (character.combat.stats.con || '').toString();
        (form.querySelector('#combat-agi') as HTMLInputElement).value = (character.combat.stats.agi || '').toString();
        (form.querySelector('#combat-lck') as HTMLInputElement).value = (character.combat.stats.lck || '').toString();
      }
      (form.querySelector('#combat-hp') as HTMLInputElement).value = (character.combat.hp || '').toString();
      (form.querySelector('#combat-mp') as HTMLInputElement).value = (character.combat.mp || '').toString();
      (form.querySelector('#combat-behavior') as HTMLInputElement).value = character.combat.behaviorType || '';
      (form.querySelector('#combat-drop-table') as HTMLInputElement).value = character.combat.dropTable || '';
    } else {
      (form.querySelector('#combat-str') as HTMLInputElement).value = '';
      (form.querySelector('#combat-mnd') as HTMLInputElement).value = '';
      (form.querySelector('#combat-con') as HTMLInputElement).value = '';
      (form.querySelector('#combat-agi') as HTMLInputElement).value = '';
      (form.querySelector('#combat-lck') as HTMLInputElement).value = '';
      (form.querySelector('#combat-hp') as HTMLInputElement).value = '';
      (form.querySelector('#combat-mp') as HTMLInputElement).value = '';
      (form.querySelector('#combat-behavior') as HTMLInputElement).value = '';
      (form.querySelector('#combat-drop-table') as HTMLInputElement).value = '';
    }

    // Optional: Combat Template
    (form.querySelector('#combat-template-id') as HTMLInputElement).value = character.combatTemplate?.id || '';

    // Optional: Sound
    if (character.sound) {
      (form.querySelector('#sound-death') as HTMLInputElement).value = character.sound.deathSound || '';
      (form.querySelector('#sound-weapon') as HTMLInputElement).value = character.sound.weaponSound || '';
    } else {
      (form.querySelector('#sound-death') as HTMLInputElement).value = '';
      (form.querySelector('#sound-weapon') as HTMLInputElement).value = '';
    }

    // Optional: Statuses
    renderStatuses(character.statuses || []);

    // Optional: Vision
    (form.querySelector('#vision-radius') as HTMLInputElement).value = (character.vision?.radius || '').toString();

    currentEditingIndex = index;
  } else {
    form.querySelector('form')?.reset();
    renderStatuses([]);
    currentEditingIndex = null;
  }
}

function renderStatuses(statuses: string[]) {
  const container = document.getElementById('statuses-list');
  if (!container) return;

  container.innerHTML = statuses
    .map(
      (status, index) => `
    <div class="status-item" data-index="${index}">
      <input type="text" class="status-input" data-index="${index}" value="${status}" placeholder="Status name" />
      <button type="button" class="btn-small btn-danger" onclick="window.removeStatus(${index})">Remove</button>
    </div>
  `
    )
    .join('');

  // Make remove function available globally
  (window as any).removeStatus = (index: number) => {
    const character = characters[currentEditingIndex!];
    if (character.statuses) {
      character.statuses.splice(index, 1);
      renderStatuses(character.statuses);
    }
  };
}

function createNewCharacter() {
  const newCharacter: CharacterTemplate = {
    type: 'TOWNSPERSON',
    name: '',
    label: '',
    spritesheet: '',
    spriteOffset: '',
    combat: {
      stats: {
        str: 1,
        mnd: 1,
        con: 1,
        agi: 1,
        lck: 1,
      },
    },
  };
  characters.push(newCharacter);
  renderCharacterList();
  renderCharacterForm(newCharacter, characters.length - 1);
  document.getElementById('character-form')?.scrollIntoView({ behavior: 'smooth' });
}

function deleteCharacter(index: number) {
  if (confirm('Are you sure you want to delete this character?')) {
    characters.splice(index, 1);
    renderCharacterList();
    if (currentEditingIndex === index) {
      renderCharacterForm(null, null);
    } else if (currentEditingIndex !== null && currentEditingIndex > index) {
      currentEditingIndex--;
    }
  }
}

function saveCharacterFromForm() {
  const form = document.getElementById('character-form')?.querySelector('form');
  if (!form) return;

  const formData = new FormData(form as HTMLFormElement);
  const character: CharacterTemplate = {
    type: formData.get('type') as string,
    name: formData.get('name') as string,
    label: formData.get('label') as string,
    spritesheet: formData.get('spritesheet') as string,
    spriteOffset: formData.get('spriteOffset') as string,
  };

  if (!character.name) {
    showNotification('Character name is required', 'error');
    return;
  }

  // Optional: Talk
  const talkName = (formData.get('talkName') as string)?.trim();
  const talkPortrait = (formData.get('talkPortrait') as string)?.trim();
  if (talkName || talkPortrait) {
    character.talk = {};
    if (talkName) character.talk.talkName = talkName;
    if (talkPortrait) character.talk.portraitName = talkPortrait;
  }

  // Optional: Combat
  const combatStr = (formData.get('combatStr') as string)?.trim();
  const combatMnd = (formData.get('combatMnd') as string)?.trim();
  const combatCon = (formData.get('combatCon') as string)?.trim();
  const combatAgi = (formData.get('combatAgi') as string)?.trim();
  const combatLck = (formData.get('combatLck') as string)?.trim();
  const combatHp = (formData.get('combatHp') as string)?.trim();
  const combatMp = (formData.get('combatMp') as string)?.trim();
  const combatBehavior = (formData.get('combatBehavior') as string)?.trim();
  const combatDropTable = (formData.get('combatDropTable') as string)?.trim();

  if (combatStr || combatMnd || combatCon || combatAgi || combatLck || combatHp || combatMp || combatBehavior || combatDropTable) {
    character.combat = {};
    
    if (combatStr || combatMnd || combatCon || combatAgi || combatLck) {
      character.combat.stats = {};
      if (combatStr) character.combat.stats.str = parseInt(combatStr);
      if (combatMnd) character.combat.stats.mnd = parseInt(combatMnd);
      if (combatCon) character.combat.stats.con = parseInt(combatCon);
      if (combatAgi) character.combat.stats.agi = parseInt(combatAgi);
      if (combatLck) character.combat.stats.lck = parseInt(combatLck);
    }
    
    if (combatHp) character.combat.hp = parseInt(combatHp);
    if (combatMp) character.combat.mp = parseInt(combatMp);
    if (combatBehavior) character.combat.behaviorType = combatBehavior;
    if (combatDropTable) character.combat.dropTable = combatDropTable;
  }

  // Optional: Combat Template
  const combatTemplateId = (formData.get('combatTemplateId') as string)?.trim();
  if (combatTemplateId) {
    character.combatTemplate = { id: combatTemplateId };
  }

  // Optional: Sound
  const soundDeath = (formData.get('soundDeath') as string)?.trim();
  const soundWeapon = (formData.get('soundWeapon') as string)?.trim();
  if (soundDeath || soundWeapon) {
    character.sound = {};
    if (soundDeath) character.sound.deathSound = soundDeath;
    if (soundWeapon) character.sound.weaponSound = soundWeapon;
  }

  // Optional: Statuses
  const statuses: string[] = [];
  document.querySelectorAll('.status-input').forEach((input) => {
    const value = (input as HTMLInputElement).value.trim();
    if (value) {
      statuses.push(value);
    }
  });
  if (statuses.length > 0) {
    character.statuses = statuses;
  }

  // Optional: Vision
  const visionRadius = (formData.get('visionRadius') as string)?.trim();
  if (visionRadius) {
    const radius = parseInt(visionRadius);
    if (!isNaN(radius)) {
      character.vision = { radius };
    }
  }

  if (currentEditingIndex !== null) {
    characters[currentEditingIndex] = character;
  } else {
    characters.push(character);
  }

  renderCharacterList();
  renderCharacterForm(null, null);
  showNotification('Character saved to list (click "Save All" to update file)', 'success');
}

function renderCharacterList() {
  const list = document.getElementById('character-list');
  if (!list) return;

  const searchTerm = (document.getElementById('search') as HTMLInputElement)?.value.toLowerCase() || '';

  const filteredCharacters = characters.filter(
    (character) =>
      character.name.toLowerCase().includes(searchTerm) ||
      character.label.toLowerCase().includes(searchTerm) ||
      character.type.toLowerCase().includes(searchTerm)
  );

  if (filteredCharacters.length === 0) {
    list.innerHTML = '<div class="empty-state">No characters found</div>';
    return;
  }

  list.innerHTML = filteredCharacters
    .map((character, index) => {
      const actualIndex = characters.indexOf(character);
      return `
      <div class="item-card ${currentEditingIndex === actualIndex ? 'editing' : ''}" onclick="window.editCharacter(${actualIndex})">
        <div class="item-card-header">
          <h3>${character.label || character.name || 'Unnamed Character'}</h3>
          <div class="item-card-actions" onclick="event.stopPropagation()">
            <button class="btn-small" onclick="window.cloneCharacter(${actualIndex})">Clone</button>
            <button class="btn-small btn-danger" onclick="window.deleteCharacter(${actualIndex})">Delete</button>
          </div>
        </div>
        <div class="item-card-body">
          <div class="item-info">
            <span class="item-type">${character.type}</span>
            <span class="item-name">${character.name}</span>
          </div>
          <p class="item-description">${character.spritesheet} @ ${character.spriteOffset}</p>
        </div>
      </div>
    `;
    })
    .join('');

  // Make functions available globally for onclick handlers
  (window as any).editCharacter = (index: number) => {
    renderCharacterForm(characters[index], index);
    document.getElementById('character-form')?.scrollIntoView({ behavior: 'smooth' });
  };
  (window as any).cloneCharacter = (index: number) => {
    const originalCharacter = characters[index];
    const clonedCharacter: CharacterTemplate = JSON.parse(JSON.stringify(originalCharacter));
    clonedCharacter.name = clonedCharacter.name + '_copy';
    characters.push(clonedCharacter);
    renderCharacterList();
    renderCharacterForm(clonedCharacter, characters.length - 1);
    document.getElementById('character-form')?.scrollIntoView({ behavior: 'smooth' });
    showNotification('Character cloned!', 'success');
  };
  (window as any).deleteCharacter = deleteCharacter;
}

export async function renderCharacterTemplatesEditor() {
  const app = document.getElementById('app');
  if (!app) return;

  app.innerHTML = `
    <div class="container">
      <div class="editor-header">
        <button class="btn-back" onclick="window.history.back()">← Back</button>
        <h1>Character Templates Editor</h1>
        <button class="btn-primary" id="save-all-btn">Save All</button>
      </div>

      <div class="editor-content">
        <div class="editor-sidebar">
          <div class="search-box">
            <input type="text" id="search" placeholder="Search characters..." />
          </div>
          <div class="item-actions">
            <button class="btn-primary" id="new-character-btn">+ New Character</button>
          </div>
          <div id="character-list" class="item-list">
            <div class="loading">Loading characters...</div>
          </div>
        </div>

        <div class="editor-main">
          <div id="character-form" class="item-form">
            <h2>${currentEditingIndex !== null ? 'Edit Character' : 'New Character'}</h2>
            <form id="character-form-element">
              <div class="form-group">
                <label for="character-type">Type *</label>
                <select id="character-type" name="type" required>
                  ${CHARACTER_TYPES.map((type) => `<option value="${type}">${type}</option>`).join('')}
                </select>
              </div>

              <div class="form-group">
                <label for="character-name">Name *</label>
                <input type="text" id="character-name" name="name" required placeholder="e.g., exampleTownsperson" />
              </div>

              <div class="form-group">
                <label for="character-label">Label *</label>
                <input type="text" id="character-label" name="label" required placeholder="e.g., Example Townsperson" />
              </div>

              <div class="form-group">
                <label for="character-spritesheet">Spritesheet *</label>
                <input type="text" id="character-spritesheet" name="spritesheet" required placeholder="e.g., characters" />
              </div>

              <div class="form-group">
                <label for="character-sprite-offset">Sprite Offset *</label>
                <input type="text" id="character-sprite-offset" name="spriteOffset" required placeholder="e.g., 0,0" />
              </div>

              <div class="form-section">
                <h3>Optional Properties</h3>

                <div class="form-subsection">
                  <h4>Talk</h4>
                  <div class="form-group">
                    <label for="talk-name">Talk Name</label>
                    <input type="text" id="talk-name" name="talkName" placeholder="e.g., talkId" />
                  </div>
                  <div class="form-group">
                    <label for="talk-portrait">Portrait Name</label>
                    <input type="text" id="talk-portrait" name="talkPortrait" placeholder="e.g., portraitSprite" />
                  </div>
                </div>

                <div class="form-subsection">
                  <h4>Combat</h4>
                  <div class="form-group">
                    <label>Stats</label>
                    <div class="form-row">
                      <div class="form-group">
                        <label for="combat-str">STR</label>
                        <input type="number" id="combat-str" name="combatStr" min="0" />
                      </div>
                      <div class="form-group">
                        <label for="combat-mnd">MND</label>
                        <input type="number" id="combat-mnd" name="combatMnd" min="0" />
                      </div>
                      <div class="form-group">
                        <label for="combat-con">CON</label>
                        <input type="number" id="combat-con" name="combatCon" min="0" />
                      </div>
                      <div class="form-group">
                        <label for="combat-agi">AGI</label>
                        <input type="number" id="combat-agi" name="combatAgi" min="0" />
                      </div>
                      <div class="form-group">
                        <label for="combat-lck">LCK</label>
                        <input type="number" id="combat-lck" name="combatLck" min="0" />
                      </div>
                    </div>
                  </div>
                  <div class="form-row">
                    <div class="form-group">
                      <label for="combat-hp">HP</label>
                      <input type="number" id="combat-hp" name="combatHp" min="0" />
                    </div>
                    <div class="form-group">
                      <label for="combat-mp">MP</label>
                      <input type="number" id="combat-mp" name="combatMp" min="0" />
                    </div>
                  </div>
                  <div class="form-group">
                    <label for="combat-behavior">Behavior Type</label>
                    <input type="text" id="combat-behavior" name="combatBehavior" placeholder="e.g., behaviorId" />
                  </div>
                  <div class="form-group">
                    <label for="combat-drop-table">Drop Table</label>
                    <input type="text" id="combat-drop-table" name="combatDropTable" placeholder="e.g., dropTableId" />
                  </div>
                </div>

                <div class="form-subsection">
                  <h4>Combat Template</h4>
                  <div class="form-group">
                    <label for="combat-template-id">Template ID</label>
                    <input type="text" id="combat-template-id" name="combatTemplateId" placeholder="e.g., templateId" />
                  </div>
                </div>

                <div class="form-subsection">
                  <h4>Sound</h4>
                  <div class="form-group">
                    <label for="sound-death">Death Sound</label>
                    <input type="text" id="sound-death" name="soundDeath" placeholder="e.g., soundName" />
                  </div>
                  <div class="form-group">
                    <label for="sound-weapon">Weapon Sound</label>
                    <input type="text" id="sound-weapon" name="soundWeapon" placeholder="e.g., soundName" />
                  </div>
                </div>

                <div class="form-subsection">
                  <h4>Statuses</h4>
                  <div id="statuses-list"></div>
                  <button type="button" class="btn-secondary" id="add-status-btn" style="margin-top: 10px;">+ Add Status</button>
                </div>

                <div class="form-subsection">
                  <h4>Vision</h4>
                  <div class="form-group">
                    <label for="vision-radius">Radius</label>
                    <input type="number" id="vision-radius" name="visionRadius" min="0" />
                  </div>
                </div>
              </div>

              <div class="form-actions">
                <button type="button" class="btn-secondary" id="cancel-btn">Cancel</button>
                <button type="submit" class="btn-primary">Save Character</button>
              </div>
            </form>
          </div>
        </div>
      </div>
    </div>
  `;

  // Load characters
  try {
    characters = await loadCharacters();
    renderCharacterList();
  } catch (error) {
    const list = document.getElementById('character-list');
    if (list) {
      list.innerHTML = `<div class="error">Error loading characters: ${error instanceof Error ? error.message : 'Unknown error'}</div>`;
    }
  }

  // Event listeners
  document.getElementById('search')?.addEventListener('input', renderCharacterList);
  document.getElementById('new-character-btn')?.addEventListener('click', createNewCharacter);
  document.getElementById('save-all-btn')?.addEventListener('click', saveCharacters);
  document.getElementById('cancel-btn')?.addEventListener('click', () => {
    renderCharacterForm(null, null);
  });
  document.getElementById('add-status-btn')?.addEventListener('click', () => {
    if (currentEditingIndex !== null) {
      const character = characters[currentEditingIndex];
      if (!character.statuses) {
        character.statuses = [];
      }
      character.statuses.push('');
      renderStatuses(character.statuses);
    } else {
      // For new characters, create a temporary structure
      const character: CharacterTemplate = {
        type: 'TOWNSPERSON',
        name: '',
        label: '',
        spritesheet: '',
        spriteOffset: '',
        statuses: [''],
      };
      characters.push(character);
      currentEditingIndex = characters.length - 1;
      renderStatuses(character.statuses);
    }
  });
  document.getElementById('character-form-element')?.addEventListener('submit', (e) => {
    e.preventDefault();
    saveCharacterFromForm();
  });
}


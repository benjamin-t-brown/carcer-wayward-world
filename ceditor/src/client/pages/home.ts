interface AssetType {
  id: string;
  name: string;
  file: string;
}

async function loadAssetTypes(): Promise<AssetType[]> {
  const response = await fetch('/api/assets/types');
  if (!response.ok) {
    throw new Error('Failed to load asset types');
  }
  return response.json();
}

export function renderHome() {
  const app = document.getElementById('app');
  if (!app) return;

  app.innerHTML = `
    <div class="container">
      <h1>Ceditor</h1>
      <div class="asset-types">
        <div class="loading">Loading asset types...</div>
      </div>
    </div>
  `;

  loadAssetTypes()
    .then((types) => {
      const typesContainer = app.querySelector('.asset-types');
      if (!typesContainer) return;

      if (types.length === 0) {
        typesContainer.innerHTML = '<div class="error">No asset types found</div>';
        return;
      }

      typesContainer.innerHTML = types
        .map(
          (type) => `
        <div class="asset-card" data-type="${type.id}">
          <h2>${type.name}</h2>
          <p>Edit ${type.name.toLowerCase()} in ${type.file}</p>
        </div>
      `
        )
        .join('');

      // Add click handlers
      document.querySelectorAll('.asset-card').forEach((card) => {
        card.addEventListener('click', () => {
          const type = (card as HTMLElement).dataset.type;
          if (type) {
            window.location.hash = `/editor/${type}`;
          }
        });
      });
    })
    .catch((error) => {
      const typesContainer = app.querySelector('.asset-types');
      if (typesContainer) {
        typesContainer.innerHTML = `<div class="error">Error: ${error instanceof Error ? error.message : 'Unknown error'}</div>`;
      }
    });
}


import '../../core/ui/base.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Home app root not found');
}

const editors = [
  ['Items', '/pages/items/'],
  ['Abilities', '/pages/abilities/'],
  ['Spells', '/pages/spells/'],
  ['Status Effects', '/pages/status-effects/'],
  ['Characters', '/pages/characters/'],
  ['Tilesets', '/pages/tilesets/'],
  ['Maps', '/pages/maps/'],
  ['Map Grids', '/pages/map-grids/'],
  ['Special Events', '/pages/events/'],
  ['Sounds', '/pages/sounds/'],
] as const;

const links = editors
  .map(([label, href]) => `<li><a href="${href}">${label}</a></li>`)
  .join('');

app.innerHTML = `
  <header>
    <h1>CEditor2</h1>
    <p>Choose a database editor.</p>
  </header>
  <nav aria-label="Database editors">
    <ul>${links}</ul>
  </nav>
`;

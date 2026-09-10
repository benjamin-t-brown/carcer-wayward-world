import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Status effects app root not found');
}

app.innerHTML = `
  <h1>Status Effects</h1>
  <p>The status-effects editor is ready to be built.</p>
`;

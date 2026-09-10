import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Sounds app root not found');
}

app.innerHTML = '<h1>Sounds</h1><p>The sounds editor is ready to be built.</p>';

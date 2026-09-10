import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Items app root not found');
}

app.innerHTML = '<h1>Items</h1><p>The items editor is ready to be built.</p>';

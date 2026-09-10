import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { StatusEffectsEditor } from './view.js';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Status effects app root not found');
}

mountDatabasePage({
  root: app,
  title: 'Status Effects',
  render(content, context) {
    const editor = new StatusEffectsEditor(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});

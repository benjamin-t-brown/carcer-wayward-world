import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { AbilitiesEditor } from './view.js';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Abilities app root not found');
}

mountDatabasePage({
  root: app,
  title: 'Abilities',
  render(content, context) {
    const editor = new AbilitiesEditor(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});

import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { SpellsEditor } from './view.js';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Spells app root not found');
}

mountDatabasePage({
  root: app,
  title: 'Spells',
  render(content, context) {
    const editor = new SpellsEditor(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});

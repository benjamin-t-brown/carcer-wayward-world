import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { ItemsEditor } from './view.js';

const app = document.querySelector<HTMLElement>('#app');

if (!app) throw new Error('Items app root not found');

mountDatabasePage({
  root: app,
  title: 'Items',
  render(content, context) {
    const editor = new ItemsEditor(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});

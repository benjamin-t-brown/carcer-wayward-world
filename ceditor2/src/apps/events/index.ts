import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { EventEditorController } from './EventEditorController.js';
import './styles.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Special events app root not found');
}

mountDatabasePage({
  root: app,
  title: 'Special Events',
  render(content, context) {
    const editor = new EventEditorController(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});

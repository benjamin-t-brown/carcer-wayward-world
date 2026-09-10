import { loadEditorData } from './bootstrap/loadEditorData';
import {
  renderBootstrapError,
  renderEditor,
  renderLoading,
} from './bootstrap/renderEditor';

async function init(): Promise<void> {
  const container = document.getElementById('root');
  if (!container) {
    console.error('Failed to initialize app: Root element not found');
    return;
  }

  renderLoading(container);
  try {
    renderEditor(container, await loadEditorData());
  } catch (error) {
    console.error('Failed to initialize app:', error);
    renderBootstrapError(container, error);
  }
}

void init();

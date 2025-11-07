import express from 'express';
import cors from 'cors';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';
import fs from 'fs/promises';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const PORT = 3001;

// Get the project root (two levels up from server directory)
const PROJECT_ROOT = join(__dirname, '../../..');
const ASSETS_DB_PATH = join(PROJECT_ROOT, 'src/assets/db');

app.use(cors());
app.use(express.json());

// Get list of asset types
app.get('/api/assets/types', async (req, res) => {
  try {
    const assetTypes = [
      {
        id: 'itemTemplates',
        name: 'Item Templates',
        file: 'items.json',
      },
      {
        id: 'characterTemplates',
        name: 'Character Templates',
        file: 'characters.json',
      },
      {
        id: 'specialEvents',
        name: 'Special Events',
        file: 'special-events.json',
      },
    ];
    res.json(assetTypes);
  } catch (error) {
    res.status(500).json({ error: 'Failed to load asset types' });
  }
});

// Load a specific asset file
app.get('/api/assets/:type', async (req, res) => {
  try {
    const { type } = req.params;
    const fileMap: Record<string, string> = {
      itemTemplates: 'items.json',
      characterTemplates: 'characters.json',
      specialEvents: 'special-events.json',
    };

    const fileName = fileMap[type];
    if (!fileName) {
      return res.status(400).json({ error: 'Invalid asset type' });
    }

    const filePath = join(ASSETS_DB_PATH, fileName);
    
    // Check if file exists
    try {
      await fs.access(filePath);
    } catch {
      // File doesn't exist, return empty array
      return res.json([]);
    }

    const content = await fs.readFile(filePath, 'utf-8');
    const data = JSON.parse(content);
    res.json(data);
  } catch (error) {
    console.error('Error loading asset:', error);
    res.status(500).json({ error: 'Failed to load asset file' });
  }
});

// Save a specific asset file
app.post('/api/assets/:type', async (req, res) => {
  try {
    const { type } = req.params;
    const fileMap: Record<string, string> = {
      itemTemplates: 'items.json',
      characterTemplates: 'characters.json',
      specialEvents: 'special-events.json',
    };

    const fileName = fileMap[type];
    if (!fileName) {
      return res.status(400).json({ error: 'Invalid asset type' });
    }

    const filePath = join(ASSETS_DB_PATH, fileName);
    
    // Ensure directory exists
    await fs.mkdir(ASSETS_DB_PATH, { recursive: true });
    
    const content = JSON.stringify(req.body, null, 2);
    await fs.writeFile(filePath, content, 'utf-8');
    res.json({ success: true });
  } catch (error) {
    console.error('Error saving asset:', error);
    res.status(500).json({ error: 'Failed to save asset file' });
  }
});

app.listen(PORT, () => {
  console.log(`Ceditor server running on http://localhost:${PORT}`);
});


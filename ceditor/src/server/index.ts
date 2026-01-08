import express from 'express';
import cors from 'cors';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';
import fs, { readdir } from 'fs/promises';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const PORT = 3001;

// Get the project root (two levels up from server directory)
const PROJECT_ROOT = join(__dirname, '../../..');
const ASSETS_DB_PATH = join(PROJECT_ROOT, 'src/assets/db');
const ASSETS_PATH = join(PROJECT_ROOT, 'src/assets');
const SRC_PATH = join(PROJECT_ROOT, 'src');

app.use(cors());
// Increase payload size limit to 50MB for large asset files
app.use(express.json({ limit: '50mb' }));


// Serve static assets (images, etc.) from src directory
app.use('/assets', express.static(SRC_PATH));

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
      {
        id: 'tilesetTemplates',
        name: 'Tileset Templates',
        file: 'tilesets.json',
      },
      {
        id: 'maps',
        name: 'Maps',
        file: 'maps.json',
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
      tilesetTemplates: 'tilesets.json',
      maps: 'maps.json',
    };

    const fileName = fileMap[type];
    if (!fileName) {
      console.error('Invalid asset type', type);
      return res.status(400).json({ error: 'Invalid asset type' });
    }

    const filePath = join(ASSETS_DB_PATH, fileName);

    // Check if file exists
    try {
      await fs.access(filePath);
    } catch {
      console.error('File does not exist', filePath);
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
      tilesetTemplates: 'tilesets.json',
      maps: 'maps.json',
    };

    console.log('saving asset', type, req.body);

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

// Get all asset files
app.get('/api/sdl2w-assets', async (req, res) => {
  // Get list of asset files
  async function getAssetFiles(assetsPath: string): Promise<string[]> {
    const files = await readdir(assetsPath);
    return files.filter((f) => f.startsWith('assets.') && f.endsWith('.txt'));
  }

  try {
    const assetFiles = await getAssetFiles(ASSETS_PATH);
    const assetFilesContent: Record<string, string> = {};
    for (const file of assetFiles) {
      const filePath = join(ASSETS_PATH, file);
      const content = await fs.readFile(filePath, 'utf-8');
      assetFilesContent[file] = content;
    }
    res.json(assetFilesContent);
  } catch (error) {
    console.error('Error listing asset files:', error);
    res.status(500).json({ error: 'Failed to list asset files' });
  }
});

// Serve sprite images from assets/img directory
// Handles URLs like: assets/img/<path>.png
app.get('/api/assets/img/*', async (req, res) => {
  try {
    // // Extract the path after /assets/img/
    // // req.path will be like '/assets/img/ui/action_buttons.png'
    // const fullPath = req.path;
    // const imagePath = fullPath.replace('/assets/img/', '');

    // // Construct the full file path
    // const filePath = join(ASSETS_PATH, 'img', imagePath);

    // // Security: Prevent directory traversal by normalizing and checking
    // const normalizedPath = join(ASSETS_PATH, 'img', imagePath);
    // const assetsImgPath = join(ASSETS_PATH, 'img');
    // if (!normalizedPath.startsWith(assetsImgPath)) {
    //   return res.status(403).json({ error: 'Invalid path' });
    // }

    const filePath = join(__dirname, '../../../src', req.path.slice(5));
    console.log('load image req.path',filePath);

    // Check if file exists
    try {
      await fs.access(filePath);
    } catch {
      console.error('Sprite image not found:', filePath);
      return res.status(404).json({ error: 'Image not found' });
    }

    // Determine content type based on file extension
    const ext = filePath.split('.').pop()?.toLowerCase();
    const contentType =
      ext === 'png'
        ? 'image/png'
        : ext === 'jpg' || ext === 'jpeg'
        ? 'image/jpeg'
        : ext === 'gif'
        ? 'image/gif'
        : ext === 'webp'
        ? 'image/webp'
        : 'application/octet-stream';

    // Read and send the file
    const imageBuffer = await fs.readFile(filePath);
    res.setHeader('Content-Type', contentType);
    res.send(imageBuffer);
  } catch (error) {
    console.error('Error serving sprite image:', error);
    res.status(500).json({ error: 'Failed to load image' });
  }
});

// Load sprites and animations from asset files
// app.get('/api/assets/sprites', async (req, res) => {
//   try {
//     const result = await loadSpritesAndAnimations(ASSETS_PATH);
//     res.json(result);
//   } catch (error) {
//     console.error('Error loading sprites:', error);
//     res.status(500).json({ error: 'Failed to load sprites' });
//   }
// });

app.listen(PORT, () => {
  console.log(`CEditor server running on http://localhost:${PORT}`);
});

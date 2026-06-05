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
app.use('/terrain-edges.html', express.static(join(__dirname + '/../client/', 'terrain-edges.html')));

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
        id: 'abilityTemplates',
        name: 'Ability Templates',
        file: 'abilities.json',
      },
      {
        id: 'statusEffectTemplates',
        name: 'Status Effect Templates',
        file: 'status-effects.json',
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
      abilityTemplates: 'abilities.json',
      statusEffectTemplates: 'status-effects.json',
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
      abilityTemplates: 'abilities.json',
      statusEffectTemplates: 'status-effects.json',
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

async function getSdl2wAssetFileContents(): Promise<Record<string, string>> {
  const files = await readdir(ASSETS_PATH);
  const assetFiles = files.filter(
    (f) => f.startsWith('assets.') && f.endsWith('.txt')
  );
  const assetFilesContent: Record<string, string> = {};
  for (const file of assetFiles) {
    const filePath = join(ASSETS_PATH, file);
    assetFilesContent[file] = await fs.readFile(filePath, 'utf-8');
  }
  return assetFilesContent;
}

// Raw SDL2W asset definition files; client parses sprites/animations via assetLoader.ts
app.get('/api/sdl2w-assets', async (req, res) => {
  try {
    res.json(await getSdl2wAssetFileContents());
  } catch (error) {
    console.error('Error listing asset files:', error);
    res.status(500).json({ error: 'Failed to list asset files' });
  }
});

function contentTypeForAssetFile(filePath: string): string {
  const ext = filePath.split('.').pop()?.toLowerCase();
  switch (ext) {
    case 'png':
      return 'image/png';
    case 'jpg':
    case 'jpeg':
      return 'image/jpeg';
    case 'gif':
      return 'image/gif';
    case 'webp':
      return 'image/webp';
    case 'wav':
      return 'audio/wav';
    case 'ogg':
      return 'audio/ogg';
    case 'mp3':
      return 'audio/mpeg';
    default:
      return 'application/octet-stream';
  }
}

async function serveSrcAssetFile(
  req: express.Request,
  res: express.Response,
  notFoundLabel: string,
) {
  const relativePath = req.path.slice(5);
  const filePath = join(SRC_PATH, relativePath);
  const normalizedPath = join(SRC_PATH, relativePath);

  if (!normalizedPath.startsWith(SRC_PATH)) {
    return res.status(403).json({ error: 'Invalid path' });
  }

  try {
    await fs.access(filePath);
  } catch {
    console.error(`${notFoundLabel} not found:`, filePath);
    return res.status(404).json({ error: `${notFoundLabel} not found` });
  }

  const fileBuffer = await fs.readFile(filePath);
  res.setHeader('Content-Type', contentTypeForAssetFile(filePath));
  res.send(fileBuffer);
}

// Serve sprite images from assets/img directory
app.get('/api/assets/img/*', async (req, res) => {
  try {
    await serveSrcAssetFile(req, res, 'Image');
  } catch (error) {
    console.error('Error serving sprite image:', error);
    res.status(500).json({ error: 'Failed to load image' });
  }
});

// Serve sound files from assets/snd directory
app.get('/api/assets/snd/*', async (req, res) => {
  try {
    await serveSrcAssetFile(req, res, 'Sound');
  } catch (error) {
    console.error('Error serving sound:', error);
    res.status(500).json({ error: 'Failed to load sound' });
  }
});

app.listen(PORT, () => {
  console.log(`CEditor server running on http://localhost:${PORT}`);
});

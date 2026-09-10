import { ASSET_IDS } from './assetRegistry.js';
import type {
  DatabaseEnvelope,
  DatabaseTransport,
  JsonObject,
  JsonValue,
  SaveDatabaseRequest,
  SaveDatabaseResponse,
} from './types.js';

export interface DatabaseClientOptions {
  baseUrl?: string;
  fetch?: typeof globalThis.fetch;
}

export class DatabaseRequestError extends Error {
  readonly status: number;
  readonly responseBody?: JsonValue;

  constructor(message: string, status: number, responseBody?: JsonValue) {
    super(message);
    this.name = 'DatabaseRequestError';
    this.status = status;
    this.responseBody = responseBody;
  }
}

export class DatabaseConflictError extends DatabaseRequestError {
  constructor(message: string, responseBody?: JsonValue) {
    super(message, 409, responseBody);
    this.name = 'DatabaseConflictError';
  }
}

export class DatabaseProtocolError extends Error {
  constructor(message: string, options?: ErrorOptions) {
    super(message, options);
    this.name = 'DatabaseProtocolError';
  }
}

export class DatabaseClient implements DatabaseTransport {
  readonly baseUrl: string;
  readonly fetch: typeof globalThis.fetch;

  constructor(options: DatabaseClientOptions = {}) {
    this.baseUrl = options.baseUrl?.replace(/\/$/, '') ?? '';
    this.fetch = options.fetch ?? globalThis.fetch?.bind(globalThis);

    if (!this.fetch) {
      throw new DatabaseProtocolError('The Fetch API is not available');
    }
  }

  async loadDatabase(): Promise<DatabaseEnvelope> {
    return parseDatabaseEnvelope(await this.request('GET'));
  }

  async saveDatabase(
    request: SaveDatabaseRequest,
  ): Promise<SaveDatabaseResponse> {
    return parseSaveDatabaseResponse(await this.request('PUT', request));
  }

  private async request(
    method: 'GET' | 'PUT',
    body?: unknown,
  ): Promise<JsonValue> {
    let response: Response;
    try {
      response = await this.fetch(`${this.baseUrl}/api/database`, {
        method,
        headers:
          body === undefined
            ? undefined
            : { 'Content-Type': 'application/json' },
        body: body === undefined ? undefined : JSON.stringify(body),
      });
    } catch (error) {
      throw new DatabaseProtocolError('Could not reach the database server', {
        cause: error,
      });
    }

    const responseBody = await readResponseBody(response);

    if (!response.ok) {
      const message = errorMessage(response, responseBody);
      if (response.status === 409) {
        throw new DatabaseConflictError(message, responseBody);
      }
      throw new DatabaseRequestError(message, response.status, responseBody);
    }

    if (responseBody === undefined) {
      throw new DatabaseProtocolError(
        `Database server returned an empty ${response.status} response`,
      );
    }

    return responseBody;
  }
}

function parseDatabaseEnvelope(value: JsonValue): DatabaseEnvelope {
  if (!isJsonObject(value) || typeof value.revision !== 'string') {
    throw new DatabaseProtocolError('Database response has no valid revision');
  }
  if (!isJsonObject(value.assets)) {
    throw new DatabaseProtocolError(
      'Database response has no valid assets object',
    );
  }
  for (const id of ASSET_IDS) {
    if (!Array.isArray(value.assets[id])) {
      throw new DatabaseProtocolError(
        `Database response has no valid ${id} collection`,
      );
    }
  }
  return value as unknown as DatabaseEnvelope;
}

function parseSaveDatabaseResponse(value: JsonValue): SaveDatabaseResponse {
  if (!isJsonObject(value) || typeof value.revision !== 'string') {
    throw new DatabaseProtocolError('Save response has no valid revision');
  }
  if (
    !Array.isArray(value.changedFiles) ||
    !value.changedFiles.every((fileName) => typeof fileName === 'string')
  ) {
    throw new DatabaseProtocolError(
      'Save response has no valid changedFiles list',
    );
  }
  return value as unknown as SaveDatabaseResponse;
}

function isJsonObject(value: JsonValue): value is JsonObject {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

async function readResponseBody(
  response: Response,
): Promise<JsonValue | undefined> {
  const text = await response.text();
  if (!text) {
    return undefined;
  }

  try {
    return JSON.parse(text) as JsonValue;
  } catch (error) {
    if (response.ok) {
      throw new DatabaseProtocolError('Database server returned invalid JSON', {
        cause: error,
      });
    }
    return text;
  }
}

function errorMessage(response: Response, body: JsonValue | undefined): string {
  if (body && typeof body === 'object' && !Array.isArray(body)) {
    const message = body.message ?? body.error;
    if (typeof message === 'string' && message.trim()) {
      return message;
    }
  }
  if (typeof body === 'string' && body.trim()) {
    return body;
  }
  return response.statusText || `Database request failed (${response.status})`;
}

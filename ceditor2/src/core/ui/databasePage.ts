import {
  ASSET_REGISTRY,
  DatabaseClient,
  DatabaseConflictError,
  DatabaseSession,
  type AssetId,
  type DatabaseTransport,
} from '../database/index.js';
import {
  validateDatabase,
  type ValidationIssue,
  type ValidationResult,
} from '../validation/index.js';
import { element } from './dom.js';

export interface DatabasePageContext {
  readonly session: DatabaseSession;
  /** Refresh shell state after changing the session. */
  notifyChanged(): void;
  /** Run client validation and save the complete database snapshot. */
  saveAll(): Promise<boolean>;
}

export type DatabasePageCleanup = () => void;

export interface DatabasePageOptions {
  root: HTMLElement;
  title: string;
  client?: DatabaseTransport;
  render(
    content: HTMLElement,
    context: DatabasePageContext,
  ): void | DatabasePageCleanup;
}

export interface ValidationSummary {
  errorCount: number;
  warningCount: number;
  text: string;
}

export function summarizeValidation(
  result: ValidationResult,
): ValidationSummary {
  const errorCount = result.errors.length;
  const warningCount = result.warnings.length;
  return {
    errorCount,
    warningCount,
    text: `${formatCount(errorCount, 'error')}, ${formatCount(warningCount, 'warning')}`,
  };
}

export function dirtyAssetLabels(ids: ReadonlySet<AssetId>): string[] {
  return ASSET_REGISTRY.filter(({ id }) => ids.has(id)).map(
    ({ label }) => label,
  );
}

export function isSaveShortcut(
  event: Pick<KeyboardEvent, 'key' | 'metaKey' | 'ctrlKey' | 'altKey'>,
): boolean {
  return (
    (event.metaKey || event.ctrlKey) &&
    !event.altKey &&
    event.key.toLowerCase() === 's'
  );
}

export function databasePageErrorMessage(error: unknown): string {
  if (error instanceof DatabaseConflictError) {
    return 'Save conflict: the database changed on disk after this page loaded. Reload the page, reapply your edits, and save again.';
  }
  if (error instanceof Error && error.message.trim()) {
    return error.message;
  }
  return 'An unexpected database error occurred.';
}

/**
 * Owns the database lifecycle shared by every independent editor page.
 * Editors only render their content and notify the shell after session changes.
 */
export class DatabasePageController {
  readonly ready: Promise<DatabaseSession>;

  private readonly options: DatabasePageOptions;
  private readonly client: DatabaseTransport;
  private readonly saveButton: HTMLButtonElement;
  private readonly dirtyStatus: HTMLElement;
  private readonly validationStatus: HTMLElement;
  private readonly operationStatus: HTMLElement;
  private readonly content: HTMLElement;
  private loadedSession?: DatabaseSession;
  private contentCleanup?: DatabasePageCleanup;
  private savePromise?: Promise<boolean>;
  private destroyed = false;
  private saving = false;

  constructor(options: DatabasePageOptions) {
    this.options = options;
    this.client = options.client ?? new DatabaseClient();

    const shell = element('div', { className: 'app-shell' });
    const header = element('header', { className: 'app-header' });
    const identity = element('div', { className: 'app-header__identity' });
    identity.append(
      element('h1', { className: 'app-header__title', text: options.title }),
    );

    const actions = element('div', { className: 'app-header__actions' });
    this.dirtyStatus = element('span', {
      className: 'database-page__dirty muted',
      text: 'Loading database…',
    });
    this.saveButton = element('button', {
      className: 'button button--primary',
      text: 'Save All',
      attributes: { type: 'button', disabled: '' },
    });
    actions.append(this.dirtyStatus, this.saveButton);
    header.append(identity, actions);

    this.validationStatus = element('details', {
      className: 'status status--info database-page__validation',
      attributes: { 'aria-live': 'polite' },
    });
    this.validationStatus.append(
      element('summary', {
        text: 'Validation will run after the database loads.',
      }),
    );
    this.operationStatus = element('div', {
      className: 'status status--info database-page__operation',
      text: 'Loading the complete game database…',
      attributes: { role: 'status', 'aria-live': 'polite' },
    });
    this.content = element('section', {
      className: 'database-page__content',
      attributes: { 'aria-busy': 'true' },
    });
    shell.append(
      header,
      this.validationStatus,
      this.operationStatus,
      this.content,
    );
    options.root.replaceChildren(shell);

    this.saveButton.addEventListener('click', this.handleSaveClick);
    window.addEventListener('keydown', this.handleKeyDown);
    window.addEventListener('beforeunload', this.handleBeforeUnload);

    this.ready = this.load();
    // The shell renders load failures even when an app does not await `ready`.
    void this.ready.catch(() => undefined);
  }

  get session(): DatabaseSession {
    if (!this.loadedSession) {
      throw new Error('The database page has not finished loading');
    }
    return this.loadedSession;
  }

  notifyChanged(): void {
    if (!this.loadedSession || this.destroyed) {
      return;
    }
    this.renderDatabaseState();
  }

  saveAll(): Promise<boolean> {
    if (this.savePromise) {
      return this.savePromise;
    }

    this.savePromise = this.performSave().finally(() => {
      this.savePromise = undefined;
    });
    return this.savePromise;
  }

  destroy(): void {
    if (this.destroyed) {
      return;
    }
    this.destroyed = true;
    this.saveButton.removeEventListener('click', this.handleSaveClick);
    window.removeEventListener('keydown', this.handleKeyDown);
    window.removeEventListener('beforeunload', this.handleBeforeUnload);
    this.contentCleanup?.();
    this.contentCleanup = undefined;
    this.options.root.replaceChildren();
  }

  private async load(): Promise<DatabaseSession> {
    try {
      const session = await DatabaseSession.load(this.client);
      if (this.destroyed) {
        return session;
      }

      this.loadedSession = session;
      this.content.removeAttribute('aria-busy');
      this.content.replaceChildren();
      const cleanup = this.options.render(this.content, {
        session,
        notifyChanged: () => this.notifyChanged(),
        saveAll: () => this.saveAll(),
      });
      this.contentCleanup = typeof cleanup === 'function' ? cleanup : undefined;
      this.operationStatus.textContent = 'Database loaded.';
      this.operationStatus.className =
        'status status--success database-page__operation';
      this.renderDatabaseState();
      return session;
    } catch (error) {
      if (!this.destroyed) {
        this.content.removeAttribute('aria-busy');
        this.operationStatus.textContent = `Could not load the database: ${databasePageErrorMessage(error)}`;
        this.operationStatus.className =
          'status status--error database-page__operation';
        this.dirtyStatus.textContent = 'Database unavailable';
        this.validationStatus.replaceChildren(
          element('summary', { text: 'Validation unavailable.' }),
        );
      }
      throw error;
    }
  }

  private async performSave(): Promise<boolean> {
    let session: DatabaseSession;
    try {
      session = await this.ready;
    } catch {
      return false;
    }
    if (this.destroyed) {
      return false;
    }
    if (!session.isDirty) {
      this.operationStatus.textContent = 'No unsaved changes.';
      this.operationStatus.className =
        'status status--info database-page__operation';
      return true;
    }

    const validation = validateDatabase(session.snapshot());
    this.renderValidation(validation);
    if (!validation.valid) {
      this.operationStatus.textContent = `Save blocked by ${formatCount(validation.errors.length, 'validation error')}.`;
      this.operationStatus.className =
        'status status--error database-page__operation';
      this.updateSaveButton(validation);
      return false;
    }

    this.saving = true;
    this.updateSaveButton(validation);
    this.operationStatus.textContent = 'Saving the complete database…';
    this.operationStatus.className =
      'status status--info database-page__operation';

    try {
      const response = await session.saveAll();
      if (this.destroyed) {
        return true;
      }
      const suffix = response.changedFiles.length
        ? ` Updated ${response.changedFiles.join(', ')}.`
        : ' Files were already up to date.';
      this.operationStatus.textContent = session.isDirty
        ? `Saved the requested snapshot.${suffix} Newer edits remain unsaved.`
        : `Database saved.${suffix}`;
      this.operationStatus.className =
        'status status--success database-page__operation';
      return true;
    } catch (error) {
      if (!this.destroyed) {
        this.operationStatus.textContent = databasePageErrorMessage(error);
        this.operationStatus.className =
          'status status--error database-page__operation';
      }
      return false;
    } finally {
      this.saving = false;
      if (!this.destroyed) {
        this.renderDatabaseState(false);
      }
    }
  }

  private renderDatabaseState(resetOperationStatus = true): void {
    const session = this.loadedSession;
    if (!session) {
      return;
    }

    const labels = dirtyAssetLabels(session.dirtyAssetIds);
    this.dirtyStatus.textContent = labels.length
      ? `Unsaved: ${labels.join(', ')}`
      : 'All changes saved';
    this.dirtyStatus.classList.toggle('muted', labels.length === 0);

    const validation = validateDatabase(session.snapshot());
    this.renderValidation(validation);
    this.updateSaveButton(validation);
    if (resetOperationStatus && labels.length) {
      this.operationStatus.textContent = 'There are unsaved changes.';
      this.operationStatus.className =
        'status status--warning database-page__operation';
    }
  }

  private renderValidation(result: ValidationResult): void {
    const summary = summarizeValidation(result);
    const summaryElement = element('summary', {
      text: `Validation: ${summary.text}.`,
    });
    const issueList = element('ul', {
      className: 'database-page__issues',
    });
    const visibleIssues = result.issues.slice(0, 20);
    for (const issue of visibleIssues) {
      issueList.append(
        element('li', {
          text: formatValidationIssue(issue),
        }),
      );
    }
    if (result.issues.length > visibleIssues.length) {
      issueList.append(
        element('li', {
          text: `${result.issues.length - visibleIssues.length} more issues not shown.`,
        }),
      );
    }
    this.validationStatus.replaceChildren(summaryElement);
    if (visibleIssues.length) {
      this.validationStatus.append(issueList);
    }
    if (summary.errorCount) {
      this.validationStatus.setAttribute('open', '');
    }
    this.validationStatus.className = `status database-page__validation ${
      summary.errorCount
        ? 'status--error'
        : summary.warningCount
          ? 'status--warning'
          : 'status--success'
    }`;
  }

  private updateSaveButton(validation: ValidationResult): void {
    const dirty = this.loadedSession?.isDirty ?? false;
    this.saveButton.disabled =
      this.saving || !dirty || validation.errors.length > 0;
    this.saveButton.textContent = this.saving ? 'Saving…' : 'Save All';
    this.saveButton.title = validation.errors.length
      ? 'Fix validation errors before saving'
      : 'Save all database files (Ctrl/Cmd+S)';
  }

  private readonly handleSaveClick = (): void => {
    void this.saveAll();
  };

  private readonly handleKeyDown = (event: KeyboardEvent): void => {
    if (!isSaveShortcut(event)) {
      return;
    }
    event.preventDefault();
    void this.saveAll();
  };

  private readonly handleBeforeUnload = (event: BeforeUnloadEvent): void => {
    if (!this.loadedSession?.isDirty) {
      return;
    }
    event.preventDefault();
    event.returnValue = '';
  };
}

export function mountDatabasePage(
  options: DatabasePageOptions,
): DatabasePageController {
  return new DatabasePageController(options);
}

function formatCount(count: number, noun: string): string {
  return `${count} ${noun}${count === 1 ? '' : 's'}`;
}

function formatValidationIssue(issue: ValidationIssue): string {
  return `${issue.severity.toUpperCase()} ${issue.path}: ${issue.message}`;
}

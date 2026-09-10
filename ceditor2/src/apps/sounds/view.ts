import {
  loadMediaCatalog,
  mediaAssetUrl,
  type SoundDefinition,
} from '../../core/media/index.js';
import { element } from '../../core/ui/dom.js';
import { matchesSoundSearch } from './editorModel.js';

export class SoundBrowser {
  private sounds: readonly SoundDefinition[] = [];
  private searchTerm = '';
  private activeAudio?: HTMLAudioElement;
  private destroyed = false;
  private readonly list = element('ul', { className: 'sound-list' });
  private readonly status = element('p', {
    className: 'status status--info',
    text: 'Loading sound catalog…',
  });

  constructor(private readonly root: HTMLElement) {}

  mount(): void {
    const search = element('input');
    search.type = 'search';
    search.placeholder = 'Sound name or path';
    search.addEventListener('input', () => {
      this.searchTerm = search.value;
      this.renderList();
    });
    const field = element('div', { className: 'field' });
    search.id = 'sound-search';
    field.append(
      element('label', {
        text: 'Search sounds',
        attributes: { for: search.id },
      }),
      search,
    );
    const header = element('section', { className: 'surface sound-controls' });
    header.append(
      field,
      element('p', {
        className: 'muted',
        text: 'Read-only catalog generated from the game asset definitions.',
      }),
      this.status,
    );
    this.root.replaceChildren(header, this.list);
    void loadMediaCatalog().then(
      (catalog) => {
        if (this.destroyed) return;
        this.sounds = [...catalog.sounds].sort((left, right) =>
          left.name.localeCompare(right.name),
        );
        this.status.className = 'status status--success';
        this.renderList();
      },
      (error: unknown) => {
        if (this.destroyed) return;
        this.status.className = 'status status--error';
        this.status.textContent = `Could not load sounds: ${error instanceof Error ? error.message : 'unknown error'}`;
      },
    );
  }

  destroy(): void {
    this.destroyed = true;
    this.stop();
    this.root.replaceChildren();
  }

  private renderList(): void {
    this.stop();
    this.list.replaceChildren();
    const matching = this.sounds.filter((sound) =>
      matchesSoundSearch(sound, this.searchTerm),
    );
    this.status.textContent = `${matching.length} of ${this.sounds.length} sounds`;
    if (!matching.length) {
      this.list.append(
        element('li', {
          className: 'empty-state',
          text: this.sounds.length
            ? 'No sounds match this search.'
            : 'No sounds were found.',
        }),
      );
      return;
    }
    for (const sound of matching) this.list.append(this.soundRow(sound));
  }

  private soundRow(sound: SoundDefinition): HTMLLIElement {
    const row = element('li', { className: 'surface sound-row' });
    const identity = element('div');
    identity.append(
      element('strong', { text: sound.name }),
      element('code', { text: sound.path }),
    );
    const play = element('button', {
      className: 'button button--small',
      text: 'Play',
      attributes: { type: 'button' },
    });
    const stop = element('button', {
      className: 'button button--small',
      text: 'Stop',
      attributes: { type: 'button' },
    });
    stop.disabled = true;
    play.addEventListener('click', () => {
      this.stop();
      const audio = new Audio(mediaAssetUrl(sound.path));
      audio.volume = sound.volume;
      this.activeAudio = audio;
      stop.disabled = false;
      const finish = () => {
        stop.disabled = true;
        if (this.activeAudio === audio) this.activeAudio = undefined;
      };
      audio.addEventListener('ended', finish, { once: true });
      audio.addEventListener(
        'error',
        () => {
          finish();
          this.status.className = 'status status--error';
          this.status.textContent = `Could not play ${sound.name} (${mediaAssetUrl(sound.path)}).`;
        },
        { once: true },
      );
      void audio.play().catch((error: unknown) => {
        finish();
        this.status.className = 'status status--error';
        this.status.textContent = `Could not play ${sound.name}: ${error instanceof Error ? error.message : 'playback failed'}`;
      });
    });
    stop.addEventListener('click', () => {
      if (this.activeAudio) this.stop();
      stop.disabled = true;
    });
    const actions = element('div', { className: 'sound-row__actions' });
    actions.append(play, stop);
    row.append(identity, actions);
    return row;
  }

  private stop(): void {
    this.activeAudio?.pause();
    if (this.activeAudio) this.activeAudio.currentTime = 0;
    this.activeAudio = undefined;
  }
}

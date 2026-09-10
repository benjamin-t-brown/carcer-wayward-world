import {
  loadSharedMediaCatalog,
  mediaAssetUrl,
  type AnimationDefinition,
  type MediaCatalog,
  type SoundDefinition,
  type SpriteDefinition,
} from '../media/index.js';
import { element } from './dom.js';
import {
  animationSpriteAt,
  mediaChoiceName,
  mediaChoices,
  type MediaPickerChoice,
  type MediaPickerKind,
} from './mediaPickerModel.js';

const RESULT_LIMIT = 240;
const imagePromises = new Map<string, Promise<HTMLImageElement>>();
const loadedImages = new Map<string, HTMLImageElement>();

export interface MediaPickerFieldOptions {
  readonly id: string;
  readonly label: string;
  readonly kind: MediaPickerKind;
  readonly value: string;
  readonly onChange: (value: string, choice?: MediaPickerChoice) => void;
  readonly help?: string;
  readonly required?: boolean;
  readonly readOnly?: boolean;
}

export function createMediaPickerField(
  options: MediaPickerFieldOptions,
): HTMLDivElement {
  const field = element('div', { className: 'field media-field' });
  const input = element('input');
  input.id = options.id;
  input.type = 'text';
  input.value = options.value;
  input.required = options.required ?? false;
  input.readOnly = options.readOnly ?? false;
  input.autocomplete = 'off';
  const preview = element('span', { className: 'media-field__preview' });
  const pick = element('button', {
    className: 'button button--small',
    text: `Choose ${options.kind}`,
    attributes: { type: 'button' },
  });
  const play = element('button', {
    className: 'button button--small',
    text: 'Play',
    attributes: { type: 'button' },
  });
  let activeAudio: HTMLAudioElement | undefined;
  let catalog: MediaCatalog | undefined;

  const renderPreview = () => {
    preview.replaceChildren();
    if (!catalog || !input.value) return;
    if (options.kind === 'sprite') {
      const sprite = catalog.spriteByName.get(input.value);
      if (sprite) preview.append(createSpritePreview(sprite, 'small'));
    } else if (options.kind === 'picture') {
      const path = catalog.pictures[input.value];
      if (path) preview.append(createPicturePreview(path));
    } else if (options.kind === 'animation') {
      const animation = catalog.animationByName.get(input.value);
      const spriteName = animation?.frames[0]?.spriteName;
      const sprite = spriteName
        ? catalog.spriteByName.get(spriteName)
        : undefined;
      if (sprite) preview.append(createSpritePreview(sprite, 'small'));
    }
    if (options.kind === 'sound') {
      play.disabled = !catalog.soundByName.has(input.value);
    }
  };

  input.addEventListener('input', () => {
    renderPreview();
    options.onChange(input.value);
  });
  pick.addEventListener('click', () => {
    if (!catalog) return;
    const picker = new MediaPickerDialog(catalog, options.kind, input.value);
    void picker.open().then((choice) => {
      if (!choice) return;
      input.value = mediaChoiceName(choice);
      renderPreview();
      options.onChange(input.value, choice);
    });
  });
  play.addEventListener('click', () => {
    if (!catalog) return;
    const sound = catalog.soundByName.get(input.value);
    if (!sound) return;
    activeAudio?.pause();
    activeAudio = playSound(sound);
  });

  const controls = element('div', { className: 'media-field__controls' });
  controls.append(preview, input, pick);
  if (options.kind === 'sound') {
    play.disabled = true;
    controls.append(play);
  }
  field.append(
    element('label', {
      text: options.label,
      attributes: { for: options.id },
    }),
    controls,
  );
  if (options.help) {
    const helpId = `${options.id}-help`;
    input.setAttribute('aria-describedby', helpId);
    field.append(
      element('span', {
        className: 'field__help',
        text: options.help,
        attributes: { id: helpId },
      }),
    );
  }

  pick.disabled = true;
  void loadSharedMediaCatalog().then(
    (loaded) => {
      if (!field.isConnected) return;
      catalog = loaded;
      pick.disabled = false;
      renderPreview();
    },
    (error: unknown) => {
      pick.title = `Media unavailable: ${error instanceof Error ? error.message : 'unknown error'}`;
    },
  );
  return field;
}

/** Small sprite used before list-card text. Unknown values get a stable blank. */
export function createEntitySpritePreview(spriteName: string): HTMLSpanElement {
  const host = element('span', {
    className: 'entity-card__media',
    attributes: { 'aria-hidden': 'true' },
  });
  void loadSharedMediaCatalog().then((catalog) => {
    if (!host.isConnected) return;
    const sprite = catalog.spriteByName.get(spriteName);
    if (sprite) host.replaceChildren(createSpritePreview(sprite, 'card'));
  });
  return host;
}

export function createEntityPicturePreview(
  pictureName: string,
): HTMLSpanElement {
  const host = element('span', {
    className: 'entity-card__media',
    attributes: { 'aria-hidden': 'true' },
  });
  void loadSharedMediaCatalog().then((catalog) => {
    if (!host.isConnected) return;
    const path = catalog.pictures[pictureName];
    if (path) host.replaceChildren(createPicturePreview(path));
  });
  return host;
}

class MediaPickerDialog {
  private readonly dialog = element('dialog', { className: 'media-picker' });
  private readonly search = element('input');
  private readonly sheet = element('select');
  private readonly results = element('div', {
    className: 'media-picker__grid',
  });
  private readonly status = element('p', { className: 'field__help' });
  private selectedName: string;
  private resolve?: (choice?: MediaPickerChoice) => void;
  private activeAudio?: HTMLAudioElement;
  private animationFrame?: number;
  private animated: Array<{
    canvas: HTMLCanvasElement;
    animation: AnimationDefinition;
  }> = [];

  constructor(
    private readonly catalog: MediaCatalog,
    private readonly kind: MediaPickerKind,
    currentValue: string,
  ) {
    this.selectedName = currentValue;
  }

  open(): Promise<MediaPickerChoice | undefined> {
    const title = `Choose ${this.kind}`;
    this.search.type = 'search';
    this.search.placeholder = `Search ${this.kind}s`;
    const controls = element('div', { className: 'media-picker__filters' });
    controls.append(this.search);
    if (this.kind === 'sprite') {
      this.sheet.append(element('option', { text: 'All pictures' }));
      const sheets = [
        ...new Set(this.catalog.sprites.map((sprite) => sprite.pictureAlias)),
      ].sort((left, right) => left.localeCompare(right));
      for (const value of sheets) {
        const option = element('option', { text: value });
        option.value = value;
        this.sheet.append(option);
      }
      const current = this.catalog.spriteByName.get(this.selectedName);
      this.sheet.value = current?.pictureAlias ?? '';
      controls.append(this.sheet);
    }
    const cancel = button('Cancel', () => this.close());
    const clear = button('Clear', () => this.close({ name: '', path: '' }));
    const use = button(
      'Use selection',
      () => this.close(this.currentChoice()),
      'button--primary',
    );
    const actions = element('div', { className: 'dialog__actions' });
    actions.append(clear, cancel, use);
    const body = element('div', { className: 'dialog__body' });
    body.append(controls, this.status, this.results);
    const header = element('div', { className: 'dialog__header' });
    header.append(element('h2', { className: 'dialog__title', text: title }));
    this.dialog.append(header, body, actions);
    document.body.append(this.dialog);
    this.search.addEventListener('input', () => this.render());
    this.sheet.addEventListener('change', () => this.render());
    this.dialog.addEventListener('cancel', (event) => {
      event.preventDefault();
      this.close();
    });
    this.dialog.addEventListener('close', () => this.finish(undefined), {
      once: true,
    });
    this.render();
    return new Promise((resolve) => {
      this.resolve = resolve;
      this.dialog.showModal();
      this.search.focus();
    });
  }

  private render(): void {
    this.stopAudio();
    if (this.animationFrame !== undefined) {
      cancelAnimationFrame(this.animationFrame);
      this.animationFrame = undefined;
    }
    this.animated = [];
    this.results.replaceChildren();
    const all = mediaChoices(
      this.catalog,
      this.kind,
      this.search.value,
      this.sheet.value,
    );
    const choices = all.slice(0, RESULT_LIMIT);
    this.status.textContent = `${all.length} match${all.length === 1 ? '' : 'es'}${all.length > RESULT_LIMIT ? ` · showing first ${RESULT_LIMIT}` : ''}`;
    for (const choice of choices) this.results.append(this.card(choice));
    if (this.kind === 'animation' && this.animated.length) this.tick(0);
  }

  private card(choice: MediaPickerChoice): HTMLDivElement {
    const name = mediaChoiceName(choice);
    const card = element('div', {
      className: 'media-picker__card',
      attributes: { 'aria-selected': String(name === this.selectedName) },
    });
    const select = element('button', {
      className: 'media-picker__choice',
      attributes: { type: 'button', title: name },
    });
    const visual = element('span', { className: 'media-picker__visual' });
    if (this.kind === 'sprite') {
      visual.append(createSpritePreview(choice as SpriteDefinition, 'picker'));
    } else if (this.kind === 'picture') {
      visual.append(createPicturePreview((choice as { path: string }).path));
    } else if (this.kind === 'animation') {
      const canvas = element('canvas', {
        className: 'media-preview media-preview--picker',
      });
      visual.append(canvas);
      this.animated.push({ canvas, animation: choice as AnimationDefinition });
    } else {
      visual.textContent = '♪';
      visual.classList.add('media-picker__visual--sound');
    }
    select.append(visual, element('span', { text: name }));
    select.addEventListener('click', () => {
      this.selectedName = name;
      for (const sibling of this.results.querySelectorAll(
        '.media-picker__card',
      )) {
        sibling.setAttribute('aria-selected', String(sibling === card));
      }
    });
    select.addEventListener('dblclick', () => this.close(choice));
    card.append(select);
    if (this.kind === 'sound') {
      card.append(
        button(
          'Play',
          () => {
            this.stopAudio();
            this.activeAudio = playSound(choice as SoundDefinition);
          },
          'button--small',
        ),
      );
    }
    return card;
  }

  private currentChoice(): MediaPickerChoice | undefined {
    return mediaChoices(this.catalog, this.kind).find(
      (choice) => mediaChoiceName(choice) === this.selectedName,
    );
  }

  private close(choice?: MediaPickerChoice): void {
    this.finish(choice);
    this.dialog.close();
  }

  private finish(choice?: MediaPickerChoice): void {
    if (!this.resolve) return;
    this.stopAudio();
    if (this.animationFrame !== undefined)
      cancelAnimationFrame(this.animationFrame);
    this.animationFrame = undefined;
    this.dialog.remove();
    const resolve = this.resolve;
    this.resolve = undefined;
    resolve(choice);
  }

  private readonly tick = (time: number): void => {
    for (const preview of this.animated) {
      const spriteName = animationSpriteAt(preview.animation, time);
      const sprite = spriteName
        ? this.catalog.spriteByName.get(spriteName)
        : undefined;
      if (sprite) drawSprite(preview.canvas, sprite);
    }
    this.animationFrame = requestAnimationFrame(this.tick);
  };

  private stopAudio(): void {
    this.activeAudio?.pause();
    this.activeAudio = undefined;
  }
}

function button(
  text: string,
  onClick: () => void,
  variant = '',
): HTMLButtonElement {
  const result = element('button', {
    className: `button${variant ? ` ${variant}` : ''}`,
    text,
    attributes: { type: 'button' },
  });
  result.addEventListener('click', onClick);
  return result;
}

function createPicturePreview(path: string): HTMLImageElement {
  const image = element('img', {
    className: 'media-preview media-preview--picture',
    attributes: { src: mediaAssetUrl(path), alt: '' },
  });
  image.loading = 'lazy';
  return image;
}

function createSpritePreview(
  sprite: SpriteDefinition,
  size: 'small' | 'card' | 'picker',
): HTMLCanvasElement {
  const canvas = element('canvas', {
    className: `media-preview media-preview--${size}`,
    attributes: { title: sprite.name },
  });
  drawSprite(canvas, sprite);
  return canvas;
}

function drawSprite(canvas: HTMLCanvasElement, sprite: SpriteDefinition): void {
  canvas.width = sprite.width;
  canvas.height = sprite.height;
  const loaded = loadedImages.get(sprite.picturePath);
  if (loaded) {
    paintSprite(canvas, sprite, loaded);
    return;
  }
  void loadImage(sprite.picturePath).then((image) => {
    if (!canvas.isConnected) return;
    paintSprite(canvas, sprite, image);
  });
}

function paintSprite(
  canvas: HTMLCanvasElement,
  sprite: SpriteDefinition,
  image: HTMLImageElement,
): void {
  const columns = Math.floor(image.naturalWidth / sprite.width);
  if (columns <= 0) return;
  const context = canvas.getContext('2d');
  if (!context) return;
  context.imageSmoothingEnabled = false;
  context.clearRect(0, 0, canvas.width, canvas.height);
  context.drawImage(
    image,
    (sprite.index % columns) * sprite.width,
    Math.floor(sprite.index / columns) * sprite.height,
    sprite.width,
    sprite.height,
    0,
    0,
    sprite.width,
    sprite.height,
  );
}

function loadImage(path: string): Promise<HTMLImageElement> {
  let promise = imagePromises.get(path);
  if (!promise) {
    promise = new Promise((resolve, reject) => {
      const image = new Image();
      image.onload = () => {
        loadedImages.set(path, image);
        resolve(image);
      };
      image.onerror = () => reject(new Error(`Could not load ${path}`));
      image.src = mediaAssetUrl(path);
    });
    imagePromises.set(path, promise);
  }
  return promise;
}

function playSound(sound: SoundDefinition): HTMLAudioElement {
  const audio = new Audio(mediaAssetUrl(sound.path));
  audio.volume = Math.min(1, Math.max(0, sound.volume));
  void audio.play().catch(() => undefined);
  return audio;
}

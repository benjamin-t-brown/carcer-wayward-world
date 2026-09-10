export function queryRequired<T extends Element>(
  selector: string,
  root: ParentNode = document,
): T {
  const result = root.querySelector<T>(selector);
  if (!result) {
    throw new Error(`Required element not found: ${selector}`);
  }
  return result;
}

export function clearElement(element: Element): void {
  element.replaceChildren();
}

export interface ElementOptions {
  className?: string;
  text?: string;
  attributes?: Readonly<Record<string, string>>;
}

export function element<K extends keyof HTMLElementTagNameMap>(
  tagName: K,
  options: ElementOptions = {},
): HTMLElementTagNameMap[K] {
  const result = document.createElement(tagName);

  if (options.className) {
    result.className = options.className;
  }
  if (options.text !== undefined) {
    result.textContent = options.text;
  }
  if (options.attributes) {
    for (const [name, value] of Object.entries(options.attributes)) {
      result.setAttribute(name, value);
    }
  }

  return result;
}

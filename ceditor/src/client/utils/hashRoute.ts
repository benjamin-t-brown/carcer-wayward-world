/** Read the current hash route (e.g. `#/editor/maps` → `/editor/maps`). */
export function readHashRoute(): {
  path: string;
  params: URLSearchParams;
} {
  const hash =
    typeof window !== 'undefined' ? window.location.hash.slice(1) || '/' : '/';
  const [path, queryString] = hash.split('?');
  return {
    path: path || '/',
    params: new URLSearchParams(queryString || ''),
  };
}

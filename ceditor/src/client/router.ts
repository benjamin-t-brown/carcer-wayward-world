type RouteHandler = () => void;

class Router {
  private routes: Map<string, RouteHandler> = new Map();
  private currentRoute: string = '';

  register(path: string, handler: RouteHandler) {
    this.routes.set(path, handler);
  }

  navigate(path: string) {
    if (this.routes.has(path)) {
      this.currentRoute = path;
      window.location.hash = path;
      this.routes.get(path)!();
    }
  }

  init() {
    // Handle browser back/forward
    window.addEventListener('popstate', () => {
      const hash = window.location.hash.slice(1) || '/';
      if (this.routes.has(hash)) {
        this.currentRoute = hash;
        this.routes.get(hash)!();
      }
    });

    // Handle initial load
    const hash = window.location.hash.slice(1) || '/';
    if (this.routes.has(hash)) {
      this.currentRoute = hash;
      this.routes.get(hash)!();
    } else {
      // Default to home
      this.navigate('/');
    }
  }

  getCurrentRoute(): string {
    return this.currentRoute;
  }
}

export const router = new Router();


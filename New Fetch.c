/**
 * api.js: frontend data layer for the real estate app.
 *
 * Load it before your main script:
 *   <script src="api.js"></script>
 *
 * Optionally set the server URL before loading (e.g. for production):
 *   <script>window.API_BASE_URL = 'https://example.com/api';</script>
 *
 * Usage:
 *   const { data, meta } = await API.getProperties({
 *     listingType: 'SALE',
 *     city: ['Nairobi', 'Mombasa', 'Meru'],
 *   });
 *
 * Cancel stale requests (search-as-you-type):
 *   const ctrl = new AbortController();
 *   API.getProperties({ q: 'villa' }, { signal: ctrl.signal });
 *   ctrl.abort();
 *
 * Handle expired sessions anywhere in the app:
 *   window.addEventListener('auth:expired', () => showLogin());
 */

const API = (() => {
  // ---------- Config ----------
  const BASE_URL = window.API_BASE_URL || 'http://localhost:5000/api';
  const TOKEN_KEY = 'auth_token';
  const TIMEOUT_MS = 10000;
  const CACHE_TTL_MS = 45000;

  // ---------- Auth token ----------
  const getToken = () => {
    try {
      return localStorage.getItem(TOKEN_KEY);
    } catch {
      return null;
    }
  };

  const setToken = (token) => {
    try {
      token ? localStorage.setItem(TOKEN_KEY, token) : localStorage.removeItem(TOKEN_KEY);
    } catch {
      /* storage unavailable: ignore */
    }
  };

  // ---------- Small GET cache ----------
  const cache = new Map(); // url string -> { expires, payload }

  const cacheGet = (key) => {
    const hit = cache.get(key);
    if (!hit) return null;
    if (hit.expires < Date.now()) {
      cache.delete(key);
      return null;
    }
    return hit.payload;
  };

  const cacheSet = (key, payload) => {
    cache.set(key, { expires: Date.now() + CACHE_TTL_MS, payload });
  };

  const clearCache = () => cache.clear();

  // ---------- Errors ----------
  function createError(message, { status = 0, details = null, type = 'http' } = {}) {
    const error = new Error(message);
    error.status = status;
    error.details = details; // e.g. field-level validation errors
    error.type = type; // 'http' | 'network' | 'timeout' | 'aborted'
    return error;
  }

  // ---------- Core request helper ----------
  async function request(
    path = '',
    { method = 'GET', body, query, signal, timeout = TIMEOUT_MS, cache: useCache = false } = {}
  ) {
    const url = new URL(BASE_URL + path);

    if (query) {
      Object.entries(query).forEach(([key, value]) => {
        if (value === undefined || value === null || value === '') return;
        if (Array.isArray(value)) {
          value
            .filter((v) => v !== undefined && v !== null && v !== '')
            .forEach((v) => url.searchParams.append(key, v));
        } else {
          url.searchParams.set(key, value);
        }
      });
    }

    const cacheKey = url.toString();
    if (useCache && method === 'GET') {
      const cached = cacheGet(cacheKey);
      if (cached) return cached;
    }

    // Headers: let the browser set Content-Type for FormData (needs the boundary)
    const headers = {};
    const isFormData = typeof FormData !== 'undefined' && body instanceof FormData;
    if (body && !isFormData) headers['Content-Type'] = 'application/json';

    const token = getToken();
    if (token) headers.Authorization = `Bearer ${token}`;

    // Timeout + optional caller-supplied abort signal
    const controller = new AbortController();
    let timedOut = false;
    const timer = setTimeout(() => {
      timedOut = true;
      controller.abort();
    }, timeout);

    if (signal) {
      if (signal.aborted) controller.abort();
      else signal.addEventListener('abort', () => controller.abort(), { once: true });
    }

    let response;
    try {
      response = await fetch(url, {
        method,
        headers,
        body: body ? (isFormData ? body : JSON.stringify(body)) : undefined,
        signal: controller.signal,
      });
    } catch (err) {
      if (err.name === 'AbortError') {
        throw timedOut
          ? createError('Request timed out. Please try again.', { type: 'timeout' })
          : createError('Request was cancelled.', { type: 'aborted' });
      }
      throw createError('Network error: could not reach the server.', { type: 'network' });
    } finally {
      clearTimeout(timer);
    }

    // 204 No Content
    let payload = null;
    if (response.status !== 204) {
      try {
        payload = await response.json();
      } catch {
        /* response had no JSON body */
      }
    }

    if (!response.ok) {
      if (response.status === 401) {
        setToken(null);
        window.dispatchEvent(new CustomEvent('auth:expired'));
      }
      throw createError(payload?.message || `Request failed (${response.status})`, {
        status: response.status,
        details: payload?.errors ?? null,
      });
    }

    if (useCache && method === 'GET') cacheSet(cacheKey, payload);
    return payload;
  }

  // ---------- Filter mapping ----------
  // Converts the UI's filter state (appState.filters) into backend query params.
  function toQuery(filters = {}) {
    const cities = [].concat(filters.cities || filters.city || []).filter(Boolean);

    return {
      listingType: filters.status === 'rent' ? 'RENT' : filters.status === 'buy' ? 'SALE' : undefined,
      propertyType: filters.type && filters.type !== 'all' ? filters.type : undefined,
      q: filters.location?.trim() || undefined,
      city: cities.length ? cities : undefined,
      maxPrice: filters.maxPrice > 0 ? filters.maxPrice : undefined,
      minBeds: filters.beds && filters.beds !== 'any' ? filters.beds : undefined,
      page: filters.page > 1 ? filters.page : undefined,
      limit: filters.limit || undefined,
      sort: filters.sort || undefined,
    };
  }

  // ---------- Public API ----------
  return {
    getToken,
    setToken,
    clearCache,
    toQuery,
    request, // exposed for new endpoints (auth, users, etc.)

    // ----- Properties -----

    // GET /properties  ->  { success, data: [...], meta: { page, limit, total, totalPages } }
    getProperties: (params = {}, opts = {}) =>
      request('/properties', { query: params, ...opts }),

    // GET /properties/:id  ->  { success, data: {...} }  (cached briefly)
    getProperty: (id, opts = {}) =>
      request(`/properties/${encodeURIComponent(id)}`, { cache: true, ...opts }),

    // POST /properties  (login required). Pass a FormData to upload images.
    createProperty: async (property, opts = {}) => {
      const result = await request('/properties', { method: 'POST', body: property, ...opts });
      clearCache();
      return result;
    },

    // PUT /properties/:id  (owner or admin)
    updateProperty: async (id, changes, opts = {}) => {
      const result = await request(`/properties/${encodeURIComponent(id)}`, {
        method: 'PUT',
        body: changes,
        ...opts,
      });
      clearCache();
      return result;
    },

    // DELETE /properties/:id  (archives the listing)
    removeProperty: async (id, opts = {}) => {
      const result = await request(`/properties/${encodeURIComponent(id)}`, {
        method: 'DELETE',
        ...opts,
      });
      clearCache();
      return result;
    },
  };
})();

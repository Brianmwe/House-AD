# House-AD
# Real Estate App: Skeleton Architecture

A single-page, vanilla HTML/CSS/JavaScript skeleton for a property listing website. It provides the page layout, application state, and function stubs for searching, filtering, saving favorites, viewing property details, and contacting an agent. No build step or framework is required.

> **Status:** This is a starter template. The structure and wiring are in place, but most functions are stubs (marked with comments) that you need to implement.

## Features

- **Navigation header** with Buy / Rent / Agents links, a saved-properties counter, and a "List Property" button
- **Search and filters** for status (sale or rent), location text, property type, max price, and bedrooms
- **Two result views:** grid cards and a map view with a sidebar list
- **Favorites drawer** to save and review properties
- **Property detail modal** with a gallery, specs, description, mortgage calculator, and agent contact form
- **Centralized state** in a single `appState` object

## Project Structure

```
.
├── index.html          # Markup and application JavaScript
├── path-to-your-styles.css   # Placeholder: replace with your stylesheet
└── properties.json     # Property data loaded at startup
```

## Getting Started

1. **Add your stylesheet.** In `index.html`, replace the placeholder in the `<link>` tag:
   ```html
   <link rel="stylesheet" href="path-to-your-styles.css">
   ```
   Your CSS should define at least a `.hidden` class (for example, `.hidden { display: none; }`), since views, the drawer, and the modal are toggled with it.

2. **Create `properties.json`** in the same folder (see [Data Format](#data-format)).

3. **Serve the folder over HTTP.** The app uses `fetch()` to load `properties.json`, which won't work when opening the file directly via `file://`. Use any local server, for example:
   ```bash
   # Python
   python3 -m http.server 8000

   # Node
   npx serve .
   ```
   Then open `http://localhost:8000`.

## Data Format

`properties.json` should contain an array of property objects. The skeleton only requires that each has a unique `id`; the other fields below are suggestions based on the UI:

```json
[
  {
    "id": 1,
    "title": "Modern 3-Bedroom Family Home",
    "status": "buy",
    "type": "house",
    "price": 350000,
    "beds": 3,
    "baths": 2,
    "location": "Nairobi",
    "lat": -1.2921,
    "lng": 36.8219,
    "description": "Bright, spacious home close to schools and shops.",
    "images": ["images/1-a.jpg", "images/1-b.jpg"]
  }
]
```

## How It Works

### State (`appState`)

| Key | Purpose |
| --- | --- |
| `properties` | Raw data loaded from `properties.json` |
| `filteredProperties` | Results after filters are applied |
| `favorites` | A `Set` of saved property IDs |
| `currentView` | `'grid'` or `'map'` |
| `filters` | `status`, `location`, `type`, `maxPrice`, `beds` |
| `activeProperty` | Property currently open in the modal |

### Flow

1. On `DOMContentLoaded`, `fetchPropertyData()` loads the data.
2. `applyFilters()` filters `appState.properties` into `filteredProperties`.
3. `renderView()` draws either the grid or the map view.
4. User actions (filter changes, favorites, opening a property) update state and re-render.

### Function Reference

| Function | Status | Description |
| --- | --- | --- |
| `fetchPropertyData()` | Implemented | Fetches `properties.json` and then calls `initFilterControls()` and `applyFilters()` |
| `setStatusFilter(status)` | Implemented | Sets sale/rent filter and re-filters |
| `handleFilterChange()` | Stub | Read filter inputs from the DOM into `appState.filters` |
| `applyFilters()` | Stub | Currently returns every property; add your filter logic |
| `switchView(mode)` | Partial | Updates state; still needs to toggle `.hidden` on the view containers |
| `renderView()` | Implemented | Routes to grid or map renderer |
| `renderPropertyGrid()` | Stub | Build and append property cards |
| `renderMapView()` | Stub | Render map pins and the sidebar list |
| `toggleFavorite(id)` | Implemented | Adds or removes a favorite and updates the counter |
| `toggleFavoritesDrawer()` | Implemented | Shows or hides the drawer |
| `renderFavoritesDrawerContent()` | Stub | Populate the drawer from `favorites` |
| `openPropertyModal(id)` | Partial | Sets `activeProperty` and shows the modal; still needs to fill in the content |
| `closePropertyModal()` | Implemented | Hides the modal |
| `runCalculator()` | Stub | Compute mortgage figures from the active property's price |
| `submitAgentInquiry(event)` | Stub | Prevents default submit; add your API call |
| `handleReset()` | Stub | Reset filters and reload the initial view |

## Known Gaps / To-Do

These items are referenced in the code but not yet defined, and will cause errors or do nothing until you add them:

- [ ] **`openListingModal()`** is called by the "List Property" button but is not defined.
- [ ] **`initFilterControls()`** is called inside the async `fetchPropertyData()` but is not defined. It should populate the property type and bedroom dropdowns and set the price slider range.
- [ ] **Duplicate `fetchPropertyData()`:** the async version (which loads `properties.json`) appears after the closing `</html>` tag. Move it inside the `<script>` block and remove the original placeholder version, otherwise it will not run.
- [ ] Implement the stubbed functions listed above.
- [ ] Add a real map integration (for example Leaflet or Google Maps) in `#map-canvas-simulator`.
- [ ] Wire `submitAgentInquiry()` and "List Property" to a backend.
- [ ] Persist favorites (for example in `localStorage`) if you want them to survive a page reload.
- [ ] Set up the Buy / Rent / Agents anchor links (`#buy`, `#rent`, `#agents`) to point to real sections or pages.

## Customization Tips

- Add new filters by adding an input in the filter grid, a matching key in `appState.filters`, and a condition in `applyFilters()`.
- Add new modal sections (floor plans, nearby amenities) inside `.modal-details` and populate them in `openPropertyModal()`.
- To use a backend instead of a static file, change the URL in `fetchPropertyData()` (for example `fetch('/api/properties')`).

## License

Add your license here (for example MIT).

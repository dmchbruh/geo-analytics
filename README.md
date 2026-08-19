# GeoAnalytics

Location intelligence engine for small-business site selection. The project aggregates OpenStreetMap points of interest into H3 hex grids, scores each zone by demand vs. competition, and visualizes opportunities on an interactive map.

Built as a C++23 backend with a lightweight web UI — suitable as a portfolio piece demonstrating geospatial data pipelines, API design, and practical analytics.

![GeoAnalytics screenshot](docs/screenshot.png)

## Highlights

- **H3 hex aggregation** at configurable resolution (default: 9)
- **Opportunity scoring**: demand weight vs. competitor penalty, adjustable in the UI
- **Growth signal**: construction / development activity as an investment indicator
- **Chain vs. independent segmentation**: competitor breakdown with visual bar in the map popup and sidebar
- **Landmarks**: nearby attractions, parks, and venues shown per hex
- **EN / RU interface**: language toggle in the top bar, switches instantly without reload
- **Top zones navigation**: click any top-10 zone in the sidebar to fly to it on the map
- **Export**: download results as GeoJSON or CSV directly from the UI
- **Dual data sources**: live Overpass API or offline `.osm.pbf` extracts via Osmium
- **REST API** + embedded map UI (Leaflet)
- **Unit tests** with Catch2

## Scoring methodology

Analysis runs in two stages.

**Stage 1 — Data collection.** For a given city and business type, three point sets are fetched from OSM:

- **Competitors** — existing businesses of the same type (e.g. cafes for a coffee shop)
- **Demand proxies** — population and footfall signals: offices, residential buildings, bus stops
- **Growth signal** — active construction sites as a forward-looking indicator

All points are projected onto an H3 hexagonal grid at resolution 9 (each cell ≈ 0.1 km²).

**Stage 2 — Scoring.** Each hex receives an opportunity score:

```
score = demand_score − competition_penalty
```

Both terms are normalised against the 90th percentile of their distributions across all hexes — this prevents a handful of dense outlier cells from collapsing the rest of the map into a single low band.

```
demand_score      = (demandCount / p90_demand) × demand_weight × 100
competition_penalty = (competitorCount / p90_competitors) × competitor_weight × 100
```

Weights default to `demand = 0.7`, `competition = 0.3` and are adjustable in the UI.

**Category assignment** uses rank percentiles rather than fixed thresholds, so the distribution is always meaningful regardless of city size:

| Category | Rank percentile |
|----------|----------------|
| VERY_HIGH | top 10% |
| HIGH | next 20% |
| MEDIUM | next 40% |
| LOW | bottom 30% |

**Investment score** is calculated separately from the growth signal (construction activity) using the same 90th-percentile normalisation, and displayed independently in each hex popup.

## Architecture

```text
Web UI (Leaflet)  →  HTTP API (httplib)  →  Analytics core
                              ↓
                    OSM / PBF data sources
                              ↓
                     H3 grid + scoring
                              ↓
                     GeoJSON + summary JSON
```

| Component | Role |
|-----------|------|
| `GeoAnalyticsServer` | HTTP server, serves UI and `/analyze` |
| `GeoAnalytics` | CLI batch pipeline (CSV or city/business mode) |
| `GeoAnalyticsCore` | Shared library: H3, scoring, I/O, geo sources |

## Requirements

- Windows 10+ (primary dev environment)
- CMake 3.21+
- MSVC with C++23
- [vcpkg](https://vcpkg.io/) packages: `h3`, `spdlog`, `nlohmann-json`, `cpr`, `httplib`, `sqlite3`, `catch2`
- **Osmium Tool** on `PATH` for offline PBF mode ([install guide](https://osmcode.org/osmium-tool/))

## Build

```powershell
cd project
cmake --preset default
cmake --build out/build/default
ctest --test-dir out/build/default
```

## Run

### Server + map UI

```powershell
cd project/out/build/default
.\GeoAnalyticsServer.exe
```

Open [http://127.0.0.1:8080/](http://127.0.0.1:8080/) if the browser does not launch automatically.

1. Enter a city (e.g. `Tbilisi`)
2. Choose business type and data source
3. Click **Analyze**

### CLI

```powershell
.\GeoAnalytics.exe --city Tbilisi --business coffee_shop
```

## Configuration

| File | Purpose |
|------|---------|
| `config/app.json` | Scoring weights, paths, H3 resolution |
| `config/pbf_sources.json` | Offline PBF files and country codes |

Example PBF entry:

```json
{
  "georgia": {
    "path": "data/pbf/georgia-latest.osm.pbf",
    "country_code": "ge"
  }
}
```

Download regional extracts from [Geofabrik](https://download.geofabrik.de/).

## API

| Endpoint | Description |
|----------|-------------|
| `GET /` | Web UI |
| `GET /health` | Health check |
| `GET /pbf-sources` | List configured offline sources |
| `GET /analyze?city=&business=&source=&demand_weight=&competitor_weight=` | Run analysis, weights in range 0.0–1.0 |

## Logging & troubleshooting

Logs are written to:

- `logs/server.log` — API server
- `logs/cli.log` — CLI runs

If the console closes unexpectedly, check the log file first. The process now waits for **Enter** before exiting so error messages remain visible.

Common issues:

| Symptom | Likely cause |
|---------|--------------|
| Console closes instantly | Fatal crash — see `logs/server.log` |
| "Could not connect to server" | `GeoAnalyticsServer.exe` is not running |
| PBF analysis fails | Osmium not installed or wrong working directory |
| City/country mismatch | Selected offline PBF does not cover the geocoded city |

## Business types

| Key | Competitors | Demand proxies | Landmarks |
|-----|-------------|----------------|-----------|
| `coffee_shop` | cafes | offices, residential, bus stops | attractions, museums, parks, malls, theatres |
| `barbershop` | hairdressers, barbers | residential, bus stops | attractions, museums, parks, malls, theatres |
| `restaurant` | restaurants, fast food | offices, residential, bus stops | attractions, museums, parks, malls, theatres, nightclubs |

## Tech stack

C++23 · H3 · SQLite · spdlog · nlohmann/json · cpr · cpp-httplib · Catch2 · Leaflet · OpenStreetMap

## License

Educational / portfolio project. OpenStreetMap data © OpenStreetMap contributors (ODbL).

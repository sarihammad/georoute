# GeoRoute Build Guide

## Dependency Strategy

GeoRoute uses a **system → vendored → fetch** dependency resolution strategy to ensure reproducible, offline-friendly builds.

### Default Behavior (Offline-First)

By default, GeoRoute builds **without network access**:

1. **System packages**: CMake first searches for system-installed packages (`find_package`)
2. **Vendored headers**: Falls back to vendored header-only libraries in `third_party/`
3. **FetchContent**: Only used if explicitly enabled (requires network)

### Dependencies

GeoRoute requires:

- **nlohmann/json** (v3.11.3): Header-only JSON library
- **cpp-httplib** (v0.15.3): Header-only HTTP server library
- **Catch2**: Test framework (only needed if building tests)

### Build Options

```bash
# Default: offline build using vendored headers
cmake -S . -B build

# Use system packages (if installed)
cmake -S . -B build
# System packages are automatically detected via find_package

# Enable FetchContent (requires network)
cmake -S . -B build -DGEOROUTE_FETCH_DEPS=ON

# Allow missing deps (for custom setups)
cmake -S . -B build -DGEOROUTE_REQUIRE_SYSTEM_DEPS=OFF
```

### Installing System Packages

#### Ubuntu/Debian

```bash
sudo apt-get install nlohmann-json3-dev
# cpp-httplib typically not in repos; use vendored or fetch
```

#### macOS (Homebrew)

```bash
brew install nlohmann-json
# cpp-httplib typically not in repos; use vendored or fetch
```

#### Manual Installation

If system packages aren't available, the vendored headers in `third_party/` are used automatically.

### Vendored Headers

The repository includes vendored header-only libraries:

- `third_party/nlohmann/json.hpp` - nlohmann/json v3.11.3
- `third_party/httplib/httplib.h` - cpp-httplib v0.15.3

These are included in the repository and allow offline builds by default.

### FetchContent (Optional)

If you prefer CMake to download dependencies at configure time:

```bash
cmake -S . -B build -DGEOROUTE_FETCH_DEPS=ON
```

**Note**: This requires network access and makes builds non-reproducible across different network conditions.

### CI/CD Considerations

For CI environments:

1. **Recommended**: Use vendored headers (default, no network needed)
2. **Alternative**: Install system packages in CI image
3. **Not recommended**: FetchContent (adds network dependency)

### Troubleshooting

**Error: "nlohmann_json not found"**

- Ensure `third_party/nlohmann/json.hpp` exists (vendored)
- Or install system package
- Or enable `-DGEOROUTE_FETCH_DEPS=ON`

**Error: "cpp-httplib not found"**

- Ensure `third_party/httplib/httplib.h` exists (vendored)
- Or enable `-DGEOROUTE_FETCH_DEPS=ON`

**Build fails offline**

- Check that `third_party/` directory contains vendored headers
- Verify `GEOROUTE_FETCH_DEPS` is OFF (default)


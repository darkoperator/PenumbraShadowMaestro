# Repository Guidelines

## Project Structure & Modules
- Root contains the Arduino sketch `PenumbraShadowMD.ino` and headers: `BoardInit.h`, `Debug.h`, `ShadowSound.h`, `pin-map.h`.
- Build config: `platformio.ini`; optional legacy `Makefile` (Arduino.mk) is present but PlatformIO is preferred.
- Assets/config: `partitions.csv`, `PenumbraSchematic.pdf`.
- No dedicated `tests/` directory; testing is device‑in‑the‑loop.

## Build, Upload, Monitor
- Build (PlatformIO): `pio run -e penumbrashadow`
- Upload (USB/serial): `pio run -e penumbrashadow -t upload`
- Serial monitor (115200): `pio device monitor -b 115200`
- Common tweaks: set `upload_port` in `platformio.ini` if auto‑detect fails; libraries are pinned in `lib_deps`.

## Coding Style & Naming
- Language: Arduino C++ (ESP32/Arduino framework).
- Indentation: 4 spaces; tabs not allowed. Max line ~100 chars.
- Braces: Allman for classes/functions (opening brace on its own line) to match existing files.
- Naming: macros/constants UPPER_SNAKE_CASE; types PascalCase; functions/methods lowerCamelCase; file names keep existing patterns (e.g., `ShadowSound.h`, `pin-map.h`).
- Comments: `//` for inline; keep doc blocks short and specific.

## Testing Guidelines
- No unit tests. Validate on hardware using the serial log and behavior.
- Enable diagnostics by toggling `USE_DEBUG` and `SHADOW_VERBOSE` macros in `PenumbraShadowMD.ino`.
- Use the PlatformIO monitor with `esp32_exception_decoder` (already configured) for crash traces.

## Commit & Pull Requests
- Commits: imperative mood, concise subject (≤72 chars), describe what/why; group related changes.
- Suggested prefixes: `feat:`, `fix:`, `perf:`, `refactor:`, `docs:`, `build:`.
- PRs: include a clear description, affected modules/files, test steps (build, upload, expected serial output), and screenshots/logs where useful. Link related issues.

## Security & Configuration Tips
- Sound backend and features are compile‑time toggles in `PenumbraShadowMD.ino` (e.g., `#define USE_MP3_TRIGGER`, `#define USE_DFMINI_PLAYER`). Only enable one sound backend at a time.
- NeoPixel defaults to GPIO 13; adjust in `pin-map.h` if needed.
- For custom boards/flash, update `partitions.csv` and `platformio.ini` (`board_build.*`, `monitor_speed`).

## Agent‑Specific Instructions
- Prefer PlatformIO tasks; avoid introducing new build systems.
- Keep patches minimal and consistent with existing style; do not reformat unrelated files.
- If you change commands or config, update `README.md` accordingly.

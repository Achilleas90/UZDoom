# Repository Guidelines

## Quick Start
1. Fork and clone your fork, then `cd` into the repo.
2. Configure and build:
   ```bash
   cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPK3_QUIET_ZIPDIR=ON
   cmake --build build --config Debug
   ```
3. Run the engine from the build output (typically `build/uzdoom`).
4. Make a focused change and format only touched code.
5. Rebuild, test the change in-game, and open a PR with testing notes.

## Project Structure & Module Organization
Core engine code lives in `src/`, split by domain (for example `src/rendering/`, `src/scripting/`, `src/playsim/`, and platform layers in `src/win32/` and `src/posix/`).
Game data and packaged resources live in `wadsrc*/` directories (`wadsrc/`, `wadsrc_bm/`, `wadsrc_extra/`, etc.).
Third-party and bundled libraries are under `libraries/`.
Build helpers and developer utilities are in `tools/` (for example `tools/zipdir/`, `tools/lemon/`, `tools/re2c/`).
Format and map specs are documented in `specs/`.

## Build, Test, and Development Commands
Use CMake with Ninja (matching CI):

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPK3_QUIET_ZIPDIR=ON
cmake --build build --config Debug
```

Switch `Debug` to `Release` or `RelWithDebInfo` as needed.
Run the built executable from the build directory (typically `build/uzdoom` on Linux/macOS).
Reconfigure from scratch when changing major options:

```bash
rm -rf build && cmake -B build -S . -G Ninja
```

## Coding Style & Naming Conventions
The project uses C++20 (`CMakeLists.txt`) and `clang-format` (`.clang-format`, Microsoft-based with project overrides).
Follow `.editorconfig`: tabs for indentation, UTF-8, LF line endings, and trimmed trailing whitespace.
Match existing file-local naming patterns rather than introducing new schemes; keep subsystem prefixes and legacy naming intact when editing older modules.
Use `clang-format` selectively on touched code, not as a repo-wide reformat.

## Testing Guidelines
There is no dedicated unit-test suite in this tree; quality gates are build health and functional verification.
Before opening a PR:
1. Build successfully on your target platform(s) with no significant new warnings.
2. Validate behavior in-game for the affected feature path.
3. Confirm packaging/resources still load when touching `wadsrc*/`, `soundfont/`, or `fm_banks/`.

## Commit & Pull Request Guidelines
Recent history favors short, imperative commit subjects (examples: `Header cleanup`, `Expand autoaim range`).
Keep commit messages focused: one logical change per commit.
PRs should include:
1. Clear summary of what changed and why.
2. Linked issue(s) when applicable.
3. Testing notes (build config + in-game checks performed).
4. Screenshots/video for UI, rendering, or visual asset changes.

For text/localization updates, use Weblate rather than direct string edits when applicable (`CONTRIBUTING.md`).

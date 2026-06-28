# Contributing to VARSYS

Thanks for your interest! A few notes to keep the project clean.

## Ground rules

- VARSYS is for **authorized security research only** — see
  [DISCLAIMER](DISCLAIMER.md). PRs that add indiscriminate‑harm payloads
  (broadband jammers, mass BLE/Apple spam, detection‑evasion for malicious use)
  will not be merged.
- Keep the build green: `pio run -e t-embed-cc1101` must pass.

## Architecture

- Each feature is a `IModule` registered in `src/core/Core.cpp`.
- UI screens derive from `Screen` and are added in `UIManager::buildScreens()`.
- Cross‑module communication goes through `EventBus`; timers via `Scheduler`.
- The cooperative loop must never block — long work belongs in a task or is
  user‑triggered and bounded. See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Style

- Match the surrounding code (naming, comments, formatting).
- Use `board_pins.h` for pins, `varsys_config.h` for tunables.
- Fonts/icons are generated — see `tools/fonts/`. Don't hand‑edit `src/ui/fonts/`.

## Commits / PRs

- Small, focused commits with clear messages.
- Note hardware that a change needs validating on, if you couldn't test it.

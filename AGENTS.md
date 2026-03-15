# Repository Guidelines

## Project Structure & Module Organization

`src/` contains the application code, split by responsibility: `core/` for app state and business logic, `sirc/` for IRC protocol handling, `sui/` for GTK UI, `render/` for message rendering, `filter/` for filtering, and `lib/` for shared utilities. Public headers live under `src/inc/`. Runtime assets are in `data/` (`ui/` Glade files, `themes/`, `icons/`, `metainfo/`). Translations are stored in `po/`. User and developer documentation is in `docs/`. Build output is typically generated in `builddir/` and local install artifacts in `prefix/`.

## Build, Test, and Development Commands

Use Meson with the helper `Makefile` for day-to-day work:

- `make` or `make build`: compile the project in `builddir/`
- `make install`: install into the local `prefix/` directory
- `make run`: run Srain with isolated `HOME` and XDG paths
- `make debug`: launch the locally installed binary under `gdb`
- `make inspect`: run with `GTK_DEBUG=interactive`
- `make clean`: remove `builddir/` and `prefix/`

For a clean setup without the helper, use `meson setup builddir` and `meson compile -C builddir`.

## Coding Style & Naming Conventions

Follow `docs/develop.rst`. Use 4 spaces, never tabs. Keep most lines under 80 columns; long strings and a few GTK-specific calls are acceptable exceptions. Opening braces stay on the same line. File names use lowercase with underscores, variables use `snake_case`, macros use uppercase, and types use `CamelCase`. Prefer declaring variables near the top of a scope. Preserve existing comments and write new comments in clear English only when needed.

## Testing Guidelines

This repository does not expose a dedicated `meson test` suite today. Validation is mainly build- and run-based: compile with `make build`, then exercise changes with `make run` or `make debug`. A few internal assertion-based tests exist in files such as `src/lib/command.c` and `src/lib/command_test.c`; keep new low-level tests close to the module they verify and name helper test functions with a `_test` suffix.

## Commit & Pull Request Guidelines

Commit messages should follow `MODULE(TYPE): DESCRIPTION`, for example `Core(fix): handle empty server name`. Recent history uses modules such as `Core`, `Sui`, `Script`, `Doc`, and `Data`. Keep descriptions imperative and specific. Pull requests should explain the user-visible change, note any platform impact, link related issues, and include screenshots when UI files under `data/ui/` or `src/sui/` are affected. Update docs or changelog entries when behavior changes.

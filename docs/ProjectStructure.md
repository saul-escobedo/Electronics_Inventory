# Project structure & file conventions

Purpose: describe how source, headers and subfolders are organized and which files are public vs internal.

- **Public headers (`include/`)**: anything placed under an `include` directory (e.g. `app/include/...`) is part of the public API and may be #included by other targets or users.
  - Use clear subfolders per component (example: `app/include/app/foo.h`).
  - Prefer `*.h` or `*.hpp` consistently across the project. Use include guards or `#pragma once`.

- **Internal implementation (`src/`)**: implementation files and non-public headers belong in `src/` (e.g. `app/src/...`). Files in `src` are private to the component and must not be included by other components directly.
  - Put helper/internal headers in `src/` or `src/detail/` so intent is clear.

- **Subproject layout** (recommended):
  - `app/`
    - `include/` — public headers for `electronics_app` (installed or consumed by other targets)
    - `src/` — .cpp files and private headers
    - `CMakeLists.txt` — builds the `electronics_app` target
  - `database/` — same pattern as `app`

- **CMake notes**:
  - Expose public headers with `target_include_directories(<target> PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)` so consumers get the include path.
  - Keep `src/` out of public include paths; link libraries expose only the public API.

- **Naming and usage rules (practical)**:
  - Public code: include with angle-brackets relative to include root, e.g. `#include <app/foo.h>`.
  - Internal code: include with quotes and relative paths inside the component, e.g. `#include "detail/helper.h"`.
  - Do not `#include` files from another component's `src/` directory.

- **Why this matters**:
  - Clear separation reduces accidental API coupling and makes refactors safer.
  - Public headers define the supported interface; internal code can change freely.

If you want, I can convert the current `app`/`database` layout to explicitly follow this pattern (add example headers/sources and update `CMakeLists.txt`).

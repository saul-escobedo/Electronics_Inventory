# Python Script Integration

This project includes a C++ to Python bridge for running external scripts and exchanging data via files or JSON payloads.

## Build Configuration

The app CMake target (`app/CMakeLists.txt`) configures:

- Python interpreter discovery: `find_package(Python3 COMPONENTS Interpreter)`
- Compile-time project root: `ECIM_PROJECT_ROOT`
- Compile-time Python executable: `ECIM_PYTHON_EXECUTABLE`

If Python cannot be discovered during configuration, the runtime fallback executable name is `python3`.

## C++ Bridge API

Header: `app/include/integration/PythonScriptRunner.hpp`
Source: `app/src/PythonScriptRunner.cpp`

Primary methods:

- `executeScript(...)`
  - Runs a Python script with arbitrary CLI args.
- `executeScriptWithFiles(...)`
  - Uses a file contract with `--input-file` and `--output-file`.
- `executeScriptWithJson(...)`
  - Serializes JSON request to a temp file and parses JSON response from a temp file.

## Script Contract

For file-based execution, scripts are expected to accept:

- `--input-file <path>`
- `--output-file <path>`

For inline JSON execution, scripts may also accept:

- `--input-json '<json>'`

A reference implementation is provided in `scripts/ecim_bridge_echo.py`.

The Digi-Key importer also supports this same contract:

- Script: `scripts/digikey_to_ecim.py`
- Input: `--input-file` JSON (keywords, auth, options)
- Output: `--output-file` ECIM import envelope JSON

Standardized schema reference:

- Schema file: `schemas/ecim.distributor-import.schema.v1.json`
- Guide: `docs/DistributorJSONSchema.md`
- C++ validator: `app/include/integration/DistributorImportSchema.hpp`

## Application Startup Example

`app/src/Application.cpp` performs a startup health-check by invoking:

- script path: `scripts/ecim_bridge_echo.py`
- IPC mode: JSON (via temporary input/output files)

The UI displays the bridge status (`OK` or `FAILED`) so script integration problems are immediately visible.

## Recommendations for New Scripts

- Return machine-readable JSON for deterministic C++ parsing.
- Keep stdout/stderr clean and reserve stderr for actionable errors.
- Use explicit exit codes (`0` success, non-zero failure).
- Validate input schema in Python before processing.

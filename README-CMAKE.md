# Build instructions

Windows (PowerShell):

```powershell
mkdir build
cmake -S . -B build
cmake --build build --config Release
```

You can disable optional subprojects by passing:

```powershell
cmake -S . -B build -DENABLE_APP=OFF -DENABLE_DATABASE=OFF
```

If you prefer a specific generator (e.g. Ninja), pass `-G "Ninja"` to `cmake`.

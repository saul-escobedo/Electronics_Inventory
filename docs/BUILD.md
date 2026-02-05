# Build & run (Windows)

1. Create build directory and configure

```powershell
mkdir build
cmake -S . -B build
```

2. Build (Release config)

```powershell
cmake --build build --config Release
```

3. Run the executable

```powershell
.\build\Release\Electronics_Inventory.exe
```

Ninja (faster incremental builds):

```powershell
# generate build files for Ninja (single-config)
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# build with ninja (parallel)
ninja -C build -j 8
# or via cmake wrapper
cmake --build build -j 8
```

Notes:
- If you used a single-config generator (Ninja/Makefiles) the EXE may be at `build\Electronics_Inventory.exe`.
- If you can't find the EXE, run:

```powershell
Get-ChildItem build -Recurse -Filter Electronics_Inventory.exe
```
- Ensure your compiler and CMake are installed and on PATH (or use a Developer Command Prompt for Visual Studio).
- Replace `8` with your core count or use `-j 0` to let Ninja decide.
# Build & run (Windows)

1. Create build directory and configure

```powershell
mkdir build
cmake -S . -B build
```

2. Build (Release config)

```powershell
cmake --build build --config Release
```

3. Run the executable

```powershell
.\build\Release\Electronics_Inventory.exe
```

Notes:
- If you used a single-config generator (Ninja/Makefiles) the EXE may be at `build\Electronics_Inventory.exe`.
- If you can't find the EXE, run:

```powershell
Get-ChildItem build -Recurse -Filter Electronics_Inventory.exe
```
- Ensure your compiler and CMake are installed and on `PATH` (or use a Developer Command Prompt for Visual Studio).

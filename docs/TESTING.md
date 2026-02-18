# Testing

Configure the project:
```powershell
cmake -S . -B build
```

Build the test target:
```powershell
cmake --build build --config Debug --target test_components
```

Run tests via CTest:
```powershell
ctest --test-dir build --build-config Debug --output-on-failure
```

Or run the executable directly to see per-assertion output:
```powershell
cmd /c build\app\Debug\test_components.exe
```

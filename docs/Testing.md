# Testing

Configure the project:
```terminal
cmake -S . -B build
```

Build the test target:
```terminal
cmake --build build --config Debug --target test_components
```

Run tests via CTest:
```terminal
ctest --test-dir build --build-config Debug --output-on-failure
```

Or run the executable directly to see per-assertion output:
```terminal
build\app\my_tests.exe
```

NEW - Running GoogleTest
```terminal
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja -C build -j 8
build\app\my_tests.exe 
```
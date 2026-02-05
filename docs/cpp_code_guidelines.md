# C++ Code Guidelines

This document lists recommended rules and best practices for C++ source and header files.

1. No `using namespace` at global scope:
   - **Rule**: Do not put `using namespace std;` (or other namespaces) in headers or global scope.
   - **Why**: Avoids name collisions and surprising symbol leakage.
   - **Alternative**: Use fully-qualified names (`std::cout`) or selective `using` declarations in implementation scope (`using std::string;` inside a function).

2. Internal identifiers and functions should be clearly marked:
   - **Rule**: Prefix internal (non-public) functions or identifiers so they are easily recognizable (the project convention: start with a single underscore, e.g. `_do_something()`).

3. Header guards:
   - **Rule**: Every header must use either `#pragma once` or a classic include guard (`#ifndef ... #define ... #endif`).

4. Do not `#include` `.cpp` files:
   - **Rule**: Only include header files. Put definitions/implementations in `.cpp` files and compile separately.

5. Prefer `enum class` to `enum`:
   - **Rule**: Use `enum class` for strongly-typed, scoped enumerations.

6. Avoid global variables when possible:
   - **Rule**: Do not use globals unless necessary. Prefer passing state explicitly or encapsulating it in a class or singleton with a clear interface.

7. Pair headers with implementation files:
   - **Rule**: Provide a header for each module exposing its API. Inline or `constexpr` functions may be defined in headers; otherwise put implementations in corresponding `.cpp` files.

8. API functions should validate parameters and return meaningful results:
   - **Rule**: Public APIs must check inputs and return a value that conveys success or error information.

9. Represent failures with expressive types (not plain `bool`):
   - **Rule**: If a function can fail, prefer expressive return types such as `enum class` error codes, `std::optional`, or `std::expected` (when available) instead of `bool`.

10. Naming for globals and static globals:
    - **Rule**: Global variables should use `g_` prefix (e.g. `g_config`). Translation-unit-local static globals should use `s_` prefix (e.g. `s_cache`).

11. Treat warnings as errors:
    - **Rule**: Enable strict warning sets and treat warnings as errors in CI/builds (e.g. `-Wall -Wextra -Werror` for GCC/Clang).

12. Keep functions short and focused:
    - **Rule**: Prefer functions of ~60 lines or fewer. If a function becomes large, split it into smaller, well-named helper functions.

13. Use smart pointers and RAII:
    - **Rule**: Prefer value semantics and RAII. Use `std::unique_ptr`/`std::shared_ptr` as ownership models when needed; avoid raw owning pointers.
 
14. Const correctness:
   - **Rule**: Mark function parameters and members `const` where appropriate to document intent and enable optimizations.

15. Pass-by-reference and move semantics:
   - **Rule**: Pass large objects as `const T&` or `T&&` for moves; prefer passing small/cheap-to-copy types by value.

16. Prefer `nullptr`:
   - **Rule**: Use `nullptr` instead of `NULL` or `0` for pointer literal clarity.

17. Use `explicit` for single-argument constructors:
   - **Rule**: Mark single-argument constructors `explicit` to avoid unintended implicit conversions.

18. Use `override` and `final`:
   - **Rule**: Annotate overridden virtual functions with `override` and use `final` where appropriate to avoid accidental mismatches.

19. Avoid C-style casts:
   - **Rule**: Prefer `static_cast`, `reinterpret_cast`, and `const_cast` with clear intent instead of C-style casts.

20. Minimize header dependencies:
   - **Rule**: Forward-declare where possible and include only what you need. In `.cpp` files include the module's own header first.
   
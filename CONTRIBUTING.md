# Contributions Guidelines

To get started on contributing the this project's codebase, read the [build instructions](./README.md/#build-instructions) to get an idea of how to setup your local development repository.

The following guidlines are similar to [PrismLauncher's guidlines](https://github.com/PrismLauncher/PrismLauncher?tab=contributing-ov-file) with some modifications.

## Code style

These are the project's conventions for C++:

- Class and type names should be formatted as `PascalCase`: `MyClass`.
- Private or protected class data members should be formatted as `camelCase` prefixed with `m_`: `m_myCounter`.
- Private or protected `static` class data members should be formatted as `camelCase` prefixed with `s_`: `s_instance`.
- Public class data members should be formatted as `camelCase` without the prefix: `dateOfBirth`.
- Public, private or protected `static const` class data members should be formatted as `SCREAMING_SNAKE_CASE`: `MAX_VALUE`.
- Class function members should be formatted as `camelCase` without a prefix: `incrementCounter`.
- Global functions and non-`const` global variables should be formatted as `camelCase` without a prefix: `globalData`.
- `const` global variables and macros should be formatted as `SCREAMING_SNAKE_CASE`: `LIGHT_GRAY`.
- enum constants should be formatted as `PascalCase`: `CamelusBactrianus`
- Avoid inventing acronyms or abbreviations especially for a name of multiple words - like `tp` for `texturePack`.
- Use space characters for indentation; the identation width is 4.

This is an example of what the code conventions look like in practice:

```c++
#define AWESOMENESS 10

constexpr double PI = 3.14159;

enum class PizzaToppings { HamAndPineapple, OreoAndKetchup };

struct Person {
    QString name;
    QDateTime dateOfBirth;

    long daysOld() const { return dateOfBirth.daysTo(QDateTime::currentDateTime()); }
};

class ImportantClass {
public:
    void incrementCounter() {
        if (m_counter + 1 > MAX_COUNTER_VALUE)
            throw std::runtime_error("Counter has reached limit!");

        ++m_counter;
    }

    int counter() const { return m_counter; }

private:
    void _do_something() {
        m_counter += 2;
    }
    
    static constexpr int MAX_COUNTER_VALUE = 100;
    int m_counter;
};

ImportantClass importantClassInstance;
```

## Best C++ Practices

The codebase of the project should follow best practices to maintain consistency.

- Do not use `using namespace` at global scope. This is to avoid name collisions, so fully-qualified names (`std::cout`) or selective `using` declarations in implementation scope (`using std::string;` inside a function) are preferred.
- Private functions should start with a single underscore (e.g. `_do_something()`).
- Every header must use either `#pragma once` (preferrably) or a classic include guard (`#ifndef ... #define ... #endif`).
- Do not `#include` `.cpp` files.
- Use `enum class` for strongly-typed, scoped enumerations.
- Do not use globals unless necessary. Prefer passing state explicitly or encapsulating it in a class or singleton with a clear interface.
- Provide a header for each module exposing its API. Inline or `constexpr` functions may be defined in headers; otherwise put implementations in corresponding `.cpp` files.
- API functions should validate parameters and return meaningful results.
- Prefer expressive return types such as `enum class` error codes, `std::optional`, or `std::expected` (when available) instead of `bool`.
- Global variables should use `g_` prefix (e.g. `g_config`). Translation-unit-local static globals should use `s_` prefix (e.g. `s_cache`).
- Prefer functions of ~60 lines or fewer. If a function becomes large, split it into smaller, well-named helper functions.
- Use `std::unique_ptr`/`std::shared_ptr` as ownership models when needed; avoid raw owning pointers.
- Mark function parameters and members `const` where appropriate to document intent and enable optimizations.
- Pass large objects as `const T&` or `T&&` for moves.
- Use `nullptr` instead of `NULL` or `0` for pointer literal clarity.
- Annotate overridden virtual functions with `override` and use `final` where appropriate to avoid accidental mismatches.
- Prefer `static_cast`, `reinterpret_cast`, and `const_cast` with clear intent instead of C-style casts.

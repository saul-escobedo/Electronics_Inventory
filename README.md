# Electronics Inventory

A desktop application built with Qt that allows users to manage an inventory of electronic components. The app provides a simple interface to add, edit, delete, and store components in a local SQLite database.

## Features

* Add new electronic components with relevant details
* Edit existing component records
* Delete components from the inventory
* Persistent local storage using SQLite
* Desktop GUI built with Qt Widgets

## Tech Stack

* **Language:** C++
* **Framework:** Qt (Widgets)
* **Database:** SQLite
* **Build System:** CMake
* **IDE (recommended):** Qt Creator

## Getting Started

### Prerequisites

* Qt (with Widgets module installed)
* CMake (3.16 or newer recommended)
* A C++ compiler (GCC, Clang, or MSVC)

### Build and Run (Qt Creator)

1. Open Qt Creator
2. Select **Open Project**
3. Choose the `CMakeLists.txt` file
4. Configure the project with your preferred kit
5. Click **Run**

### Build and Run (Command Line)

```bash
git clone https://github.com/saul-escobedo/Electronics_Inventory.git
cd Electronics_Inventory
git checkout saul-ui

mkdir build
cd build
cmake ..
cmake --build .
```

Then run the generated executable.

## Project Structure

```
Electronics_Inventory/
├── src/            # Application source files
├── include/        # Header files
├── ui/             # UI forms (Qt Designer)
├── CMakeLists.txt  # Build configuration
└── README.md
```

## Database

The application uses a local SQLite database file to persist component data.
The database is created/used automatically when the application runs.

## Future Improvements

* Search and filtering functionality
* Sorting components by category or attributes
* Export/import inventory (CSV, JSON)
* Improved UI/UX enhancements
* Validation and error handling

## Screenshots

*Add screenshots of the UI here to showcase the application.*

## Author

Saul Escobedo

## License

This project is open source and available under the MIT License.

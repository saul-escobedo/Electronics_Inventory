# UI Development

Since this project uses Qt for the user interface, QtCreator can be used to quickly iterate over the interface such as the layout and styling. We highly recommended using QtCreator when developing the UI for this project since developing it progmatically can be a headache to do.

## Using QtCreator With this Project

Ensure you have QtCreator installed. On systems, like Debian/Ubuntu, you'd typically install it using the package manager:

```bash
sudo apt install qtcreator
```

Open QtCreator, and head to `File > Open File or Project`.

Navigate to and select `CMakeLists.txt` in this project's root directory.

You'd typically only do Debug builds while developing the UI, so under the `Desktop` dropdown, uncheck other build types besides `Debug` and set the build directory to `<project_path>/build` (replace `<project_path>` to where this repository is stored locally).

Finally, click `Configure Project`. You can now edit the layout of the UI by editing `.ui` files at `ecim/app/ecim_impl` in the project hierarchy.

**Note:**

You may use your IDE of choice for programming other stuff (e.g., the backend) while using QtCreator for the UI, but to avoid confusion, make sure your IDE uses this command to build the project:

```bash
cmake --build . --target ecim
```

instead of:

```bash
make ecim
```

This is because QtCreator uses another underlying build system other than *Makefile* to construct the project.

## Files

As mentioned in the [README](../README.md), the primary codebase resides in the `app` directory. Inside of it, you'll find all the UI files. Specifically:
* `.hpp` header files of custom Qt widgets reside in `app/include/ui`
* `.ui` layout files reside in `app/layout`
* `.cpp` source files of custom Qt widgets reside in `app/src/ui`

The `.ui` files are edited in QtCreator, not by hand.

## Recommended: Prepopulating the SQLite Database For Development

Inside `scripts/filldb` resides the `filldb.sh` bash script that allows the developer to prepopulate an sqlite database for development. It will populate with real world data from [LCSC](https://www.lcsc.com/) for every type of electrical component. This is useful for testing the UI's view of items and seeing if it is populating correctly or for stress testing the backend. If working on Windows, run the script with *Git Bash* to work around the fact that `filldb.sh` is a shell script. To prepopulate an sqlite database, you'd typically want to target the database file that the application will read, like:

```bash
scripts/filldb/filldb.sh ~/.local/share/ecim/inventory.db
```

Afterwards, the database will be loaded with real world data.

### Where Is My Database File Located?

The application will print out the location of the database file to the console. After [building](../README.md#build-instructions) the project, run:

```bash
./ecim
```

On Windows, run it in powershell:

```powershell
.\ecim.exe
```

And the application will print something like:

```
DB Path:  "/home/user/.local/share/ecim/inventory.db"
Database opened successfully
Aut-backup disabled.
```

Use can use this directory with `filldb.sh`.

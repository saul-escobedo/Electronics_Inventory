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

The `.ui` files are usually edited in QtCreator, not by hand.

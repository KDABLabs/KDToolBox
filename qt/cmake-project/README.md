# cmake-project

This project contains a shell script that generates a `CMakeLists.txt`
based on the .cpp / .ui / .rcc files in the current directory.

It's the CMake equivalent of `qmake -project`.

It supports Qt 5, Qt 6 as well not using Qt at all. For instance:

```bash
cmake-project --qt 6
```

# Includemocs

When you use Q_OBJECT in a header file a moc file will be generated
which needs to be compiled with the rest of your project.

If you are using CMake, you will likely have a line like this in your
CMakeLists.txt file:

> set(CMAKE_AUTOMOC ON)

This results in all moc files being included from one file, namely mocs_compilation.cpp.
This has the advantage that it speeds up the initial build drastically, but the disadvantage that
incremental builds - when you e.g. touch just a single header file - will be much slower.

Including the moc files in the source files implementing the header file, gives us the best
of both worlds - both fast initial build and fast incremental builds.

This script does exactly that.

Run it using -h for details.

## git-hook

The file *check-includemocs-hook.sh* is a git hook to ensure all files which need it have moc files included.

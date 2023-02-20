# Qt6 Natvis

This repository is a test suite for my changes to the official Qt6 natvis file that can be found here: [qt6.natvis](https://code.qt.io/cgit/qt-labs/vstools.git/plain/QtMSBuild/QtMsBuild/qt6.natvis.xml).

Note that I removed the `##NAMESPACE##::` from the type name, as this file is parsed and updated by the [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2022).

# Changes

## QMap

|          `QMap` before          |          `QMap` after          |
|:-------------------------------:|:------------------------------:|
| ![QMap](assets/qmap-before.png) | ![QMap](assets/qmap-after.png) |
|   Comparison with `std::map`    |  ![std::map](assets/map.png)   |

## QHash

|            `QHash` before            |                  `QHash` after                  |
|:------------------------------------:|:-----------------------------------------------:|
|  ![QHash](assets/qhash-before.png)   |        ![QHash](assets/qhash-after.png)         |
| Comparison with `std::unordered_map` | ![std::unordered_map](assets/unordered_map.png) |

# QMultiHash

| `QMultiHash` before (with `QHash` improvements) |                    `QMultiHash` after                     |
|:-----------------------------------------------:|:---------------------------------------------------------:|
|   ![QMultiHash](assets/qmultihash-before.png)   |        ![QMultiHash](assets/qmultihash-after.png)         |
|    Comparison with `std::unordered_multimap`    | ![std::unordered_multimap](assets/unordered_multimap.png) |

There's a difference between `QMultiHash` and `std::unordered_multimap`, it's not possible to display `QMultiMap` as it's std equivalent. Note that the current representation is closest to the `QMultiHash` API anyway: it's a list of `[key, list<value>]` (so size is 2 here), not a list of `[key, value]` (with a size of 4).

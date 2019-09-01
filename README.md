KDToolBox
=========

KDAB's collection of miscellaneous useful C++ classes and stuff.

Tools
=================
- [Ui Watchdog](https://github.com/KDAB/KDToolBox/tree/master/qt/ui_watchdog)
  An _header-only_ tool to break the program when the main-thread event loop is blocked for more than 300ms.
- [Qml Stacktrace Helper](https://github.com/KDAB/KDToolBox/tree/master/qt/qml/QmlStackTraceHelper)
  A function for retrieving a QML backtrace with gdb.
- [Qml PropertySelector](https://github.com/KDAB/KDToolBox/tree/master/qt/qml/PropertySelector)
  A QML Item to easily set property values based on a combination of conditions
- [Model/View ModelIterator](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/modelIterator)
  A set of (template) classes to allow iteratating over QAbstractItemModels using std algorithms
- [Model/View SortProxyModel](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/sortProxyModel)
  A QSortFilterProxyModel replacement that just does sorting, but properly signals moves due to sorts.
- [Model/View UpdateableModel](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/updateableModel)
  A template class to make defining a model that sends proper update signals easy
- [Model/View Table to List Proxy](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/KDTableToListProxyModel)
  A proxy model flattening a table model into a list model (mainly for Qt Quick 2).


Other code snippets
===================
- [Integrating Qt Quick 2 with OpenGL](https://github.com/KDAB/integrating-qq2-with-opengl)
  Code for Giuseppe D'Angelo's talk at the Qt World Summit 2015, QtCon 2016, Qt World Summit 2017.


Licensing
=========

This software is provided as MIT (see the LICENSE file in the root of this repository [1]).

For the "Other code snippets", which are hosted outside of this repo, please consult
their own license.

[1] [License file] (https://github.com/KDAB/KDToolBox/blob/master/LICENSE)

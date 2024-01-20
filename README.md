# KDToolBox

KDAB's collection of miscellaneous useful C++ classes and stuff.

## Tools List

### Generic Tools

* [asan_assert_fail](https://github.com/KDAB/KDToolBox/tree/master/general/asan_assert_fail)
  Intercept `cassert` failures with ASAN and print a backtrace.

### C++ Tools

* [future-backports](https://github.com/KDAB/KDToolBox/tree/master/cpp/future-backports)
  Backports of future C++ library components to C++11:
  * k20::erase/k20::erase_if - C++20's uniform container erasure
* [DuplicateTracker](https://github.com/KDAB/KDToolBox/tree/master/cpp/duplicatetracker)
  A helper class to keep track of "seen" elements, e.g. to avoid processing duplicate elements
  in a collection. It transparently uses `std::pmr::monotonic_buffer_resource` to improve
  its memory usage (compared to a 'plain' set implementation).
* [toContainer](https://github.com/KDAB/KDToolBox/tree/master/cpp/toContainer)
  Helper functions to convert a container to another container, using a convenient pipeline style.
* [propagate_const](https://github.com/KDAB/KDToolBox/tree/master/cpp/propagate_const)
  A backport of `propagate_const` from the C++ Extensions for Library Fundamentals version 3.

### Qt Tools

* [Ui Watchdog](https://github.com/KDAB/KDToolBox/tree/master/qt/ui_watchdog)
  A _header-only_ tool to break the program when the main-thread event loop is blocked for more
  than 300ms.
* [Qml Stacktrace Helper](https://github.com/KDAB/KDToolBox/tree/master/qt/qml/QmlStackTraceHelper)
  A function for retrieving a QML backtrace with gdb.
* [Qml PropertySelector](https://github.com/KDAB/KDToolBox/tree/master/qt/qml/PropertySelector)
  A QML Item to easily set property values based on a combination of conditions
* [Model/View ModelIterator](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/ModelIterator)
  A set of (template) classes to allow iteratating over QAbstractItemModels using std algorithms
* [Model/View SortProxyModel](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/sortProxyModel)
  A QSortFilterProxyModel replacement that just does sorting, but properly signals moves due to sorts.
* [Model/View UpdateableModel](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/updateableModel)
  A template class to make defining a model that sends proper update signals easy
* [Model/View Table to List Proxy](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/KDTableToListProxyModel)
  A proxy model flattening a table model into a list model (mainly for Qt Quick 2).
* [Functional Sort/Filter Proxy](https://github.com/KDAB/KDToolBox/tree/master/qt/model_view/KDFunctionalSortFilterProxyModel)
  A QSortFilterProxyModel convenience subclass that allows to set filtering/sorting functions,
  removing the need for subclassing.
* [TabWindow widget](https://github.com/KDAB/KDToolBox/tree/master/qt/tabWindow)
  A tab widget that allows extracting a tab to a new window, like modern web browsers.
* [EventFilter](https://github.com/KDAB/KDToolBox/tree/master/qt/eventfilter)
  A way for "connecting" a QEvent to lambda with two lines of code.
* [MessageHandler](https://github.com/KDAB/KDToolBox/tree/master/qt/messagehandler)
  An API to help you intercept the output going through qDebug()/qWarning()/etc.
* [QStringTokenizer](https://github.com/KDAB/KDToolBox/tree/master/qt/stringtokenizer)
  A universal, safe, zero-allocation string splitter.
* [Signal/Slot Connection Throttlers](https://github.com/KDAB/KDToolBox/tree/master/qt/KDSignalThrottler)
  An implementation of throttler/debouncer classes for signal/slot connections.
* [Single Shot Connect](https://github.com/KDAB/KDToolBox/tree/master/qt/singleshot_connect)
  A header only version of QObject::connect, that establishes a single shot connection.
* [Qt Hasher](https://github.com/KDAB/KDToolBox/tree/master/qt/qt_hasher)
  A header-only hasher object for using Qt types together with `unordered_map`, `unordered_set`, etc.
* [NotifyGuard](https://github.com/KDAB/KDToolBox/tree/master/qt/notify_guard)
  A RAII guard object to make sure object notification signals are properly emitted.
* [asan_assert_fail_qt](https://github.com/KDAB/KDToolBox/tree/master/qt/asan_assert_fail_qt)
  Intercept `Q_ASSERT` failures with ASAN and print a backtrace.
* [KDSqlDatabaseTransaction](https://github.com/KDAB/KDToolBox/tree/master/qt/KDSqlDatabaseTransaction)
  A RAII wrapper for database transactions when using the Qt SQL APIs.
* [pointer_cast](https://github.com/KDAB/KDToolBox/tree/master/qt/pointer_cast)
  Tools to ease migrating from `QSharedPointer` to `std::shared_ptr`.
* [qt_fmt](https://github.com/KDAB/KDToolBox/tree/master/qt/qt_fmt)
  Convenience for printing Qt classes through [libfmt](https://github.com/fmtlib/fmt).
* [cmake-project](https://github.com/KDAB/KDToolBox/tree/master/qt/cmake-project)
  A replacement for `qmake -project`, generating a ready-to-use CMakeLists.txt.
* [includemocs](https://github.com/KDAB/KDToolBox/tree/master/qt/includemocs)
  A script to add inclusion of moc files to all .cpp files in the project.
* [qt6_natvis](https://github.com/KDAB/KDToolBox/tree/master/qt/qt6_natvis)
  Natvis file for Qt6 debugging with Visual Studio and VS Code, with a test project.
* [KDStlContainerAdaptor](https://github.com/KDAB/KDToolBox/tree/master/qt/KDStlContainerAdaptor)
  Classes to ease the transition from Qt containers to the C++ Standard Library containers.

### Other Code Snippets

* [Integrating Qt Quick 2 with OpenGL](https://github.com/KDAB/integrating-qq2-with-opengl)
  Code for Giuseppe D'Angelo's talk at the Qt World Summit 2015, QtCon 2016, Qt World Summit 2017.

### Squish Tools

* [kdrunsquish.py](https://github.com/KDAB/KDToolBox/tree/master/squish/kdrunsquish.py)
  Runs squish tests in parallel via the Qt offscreen QPA.

## Licensing

KDToolBox is © Klarälvdalens Datakonsult AB (KDAB) and is made available
under the terms of the [MIT](LICENSES/MIT.txt) license.

Contact KDAB at <info@kdab.com> if you need different licensing options.

### Licensing Exceptions

The [qt6_natvis](https://github.com/KDAB/KDToolBox/tree/master/qt/qt6_natvis) project is available under
the terms of the [The Qt Company GPL Exception for GPL3 license](LICENSES/LicenseRef-Qt-GPL-3.0_EXCEPTION.txt).

For the "Other code snippets", which are hosted outside of this repo, please consult
their own license.

## Get Involved

KDAB will happily accept external contributions.

Please submit your contributions or issue reports from our GitHub space at
<https://github.com/KDAB/KDToolBox>.

## About KDAB

KDToolBox is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and
OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications
from scratch and in porting from all popular and legacy frameworks to Qt.
We continue to help develop parts of Qt and are one of the major contributors
to the Qt Project. We can give advanced or standard trainings anywhere
around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit <https://www.kdab.com> to meet the people who write code like this.

Stay up-to-date with KDAB product announcements:

* [KDAB Newsletter](https://news.kdab.com)
* [KDAB Blogs](https://www.kdab.com/category/blogs)
* [KDAB on Twitter](https://twitter.com/KDABQt)

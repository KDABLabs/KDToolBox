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
KDToolBox is (C) 2019, Klarälvdalens Datakonsult AB, and is available under the
terms of the MIT license (see the LICENSE file in the root of this repository [1]).

For the "Other code snippets", which are hosted outside of this repo, please consult
their own license.

Contact KDAB at <info@kdab.com> if you need different licensing options.

[1] [License file] (https://github.com/KDAB/KDToolBox/blob/master/LICENSE)

Get Involved
============
KDAB will happily accept external contributions; however, **all**
contributions will require a signed Contributor License Agreement
(see docs/KDToolBox-CopyrightAssignmentForm.docx).

Contact info@kdab.com for more information.

About KDAB
==========
KDToolBox is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and
OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications
from scratch and in porting from all popular and legacy frameworks to Qt.
We continue to help develop parts of Qt and are one of the major contributors
to the Qt Project. We can give advanced or standard trainings anywhere
around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit http://www.kdab.com to meet the people who write code like this.

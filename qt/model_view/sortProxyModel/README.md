# SortProxyModel

SortProxyModel is a drop-in replacement for QSortFilterProxyModel in so far as
it touches the sorting capabilities. It duplicates most of the sort-related
API from its Qt counterpart. Where it differs is that unlike
QSortFilterProxyModel, SortProxyModel emits the row move signals for QML to
animate the reordering if the ordering criteria change.

## Usage

Just like you would use QSortFilterProxyModel, only without the filtering. If
you also need filtering, you will need a stack like `underlyingModel <->
QSortFilterProxyModel <-> SortProxyModel <-> View/QML'

Add the SortProxyModel.h and SortProxyModel.cpp to your application sources,
and the unit test in `test/` to your unit tests.

## Limitations

SortProxyModel does currently not support sorting tree models. Only the root
level of the underlying model is sorted. Furthermore, the model is always
sorting. There is no unsorted pass-through mode where the model shows the
order of the underlying model. Also, sorting only happens by row, not by
column.

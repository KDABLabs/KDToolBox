# Functional Sort/Filter Proxy Model

This is a convenience `QSortFilterProxyModel` subclass that allows to
filter rows/columns using a functional predicate, removing the need to subclass.

```cpp
KDFunctionalSortFilterProxyModel *proxy = new KDFunctionalSortFilterProxyModel;

auto acceptFunction = [](const QAbstractItemModel *model, int source_row, const QModelIndex &parent)
{
    // decide whether to accept `source_row` from `model`, under `parent`;
    // return true to accept, false to filter the row out
};

proxy->setFilterAcceptsRowFunction(acceptFunction);
proxy->setSourceModel(sourceModel);

view->setModel(proxy);
```

Similarly, you can set a sorting predicate:

```cpp
KDFunctionalSortFilterProxyModel *proxy = new KDFunctionalSortFilterProxyModel;

auto lessThanFunction = [](const QModelIndex &lhs, const QModelIndex &rhs)
{
    // return true if `lhs` should be sorted before `rhs`
};

proxy->setLessThanFunction(lessThanFunction);
proxy->setSourceModel(sourceModel);

view->setModel(proxy);
```

Note that when a sorting predicate is set `KDFunctionalSortFilterProxyModel`
will *not* fall back to `QSortFilterProxyModel` default implementation.

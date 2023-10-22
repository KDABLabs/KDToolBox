# Table to List Proxy Model

KDTableToListProxyModel provides a convenient proxy model flattening a table model into a list model.

The main use-case is Qt Quick 2: many of its views only support list models, and (ab)use multiple
roles for transferring multiple pieces of information from the model to the view. In case we already
have a table model, we need to flatten it

## Usage

KDTableToListProxyModel is set up like an ordinary proxy model:

```cpp
    KDTableToListProxyModel tableToListProxyModel;
    tableToListProxyModel.setSourceModel(&sourceModel);
```

Then we need to specify a mapping from roles in KDTableToListProxyModel to columns (and optionally
roles) in the source model. KDTableToListProxyModel has only one column, and the same amount of rows
as the source model.

```cpp
    // map column 0 in the source to Qt::UserRole on this model
    tableToListProxyModel.setRoleMapping(0, Qt::UserRole, "name");

    // map column 1 in the source to Qt::UserRole + 1 on this model
    tableToListProxyModel.setRoleMapping(1, Qt::UserRole + 1, "picture");
```

Role names can be provided, for extra convenience when using Qt Quick.

KDTableToListProxyModel does not support tree models (it will only show elements directly below the root).

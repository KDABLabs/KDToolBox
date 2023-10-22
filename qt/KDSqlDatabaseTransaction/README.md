KDSqlDatabaseTransaction
========================

A simple RAII wrapper for transaction support when using the Qt SQL
database classes (QSqlQuery, QSqlDatabase, and so on).

```cpp
KDSqlDatabaseTransaction transaction(dbConnection); // begins the transaction

// ... do database work ...

if (!transaction.commit()) // ends the transaction
    qWarning() << "Error!";
```

Destroying a `KDSqlDatabaseTransaction` without committing the
transaction will automatically roll it back. (It's also possible to
call `rollback()`.)

Requires C++14.

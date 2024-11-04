KDCoro (required C++23)
========================

Some awaitable classes to allow for safe usage of coroutines in C++23

```cpp
void MyDialog::save()
{
    auto coroutine = [this]() -> KDCoroTerminator {
        // Tell KDCoroTerminator that this coroutine should be destroyed
        // if MyDialog is destroyed.
        co_yeild this;

        // Wait thill the input dialog
        std::expected<QString, QString> filepath = co_await inputDialog.filenameCoroutine();

        if (!filepath.has_value()) {
            qDebug() << "Input dialog did not provide a finepath" << filepath.error();
            co_return;
        }

        QFile file(filepath);
        // save information

        MyDialog::accepted();
    };

    coroutine(); // co_await is forbidden
}
```

Another coroutine helper, is the ability to transform any signal into an awaitable:

```cpp
void MyDialog::save()
{
    auto coroutine = [this]() -> KDCoroTerminator {
        // Tell KDCoroTerminator that this coroutine should be destroyed
        // if MyDialog is destroyed.
        co_yeild this;

        // Wait thill the input dialog
        // Note that "this" was passed to kdCoroSignal(), this allows
        // this awaitable to return an std::unexpected in case MyDialog is
        // destroyed.
        // While we already do "co_yeild this;", passing "this" is the only way to
        // finish this coroutine if filepath is not emitted on an rejected dialog.
        // Care must be takend that either inputDialog or this are destroyed or that
        // the emitted guarantees to emit it's signal.
        std::expected<QString, QString> filepath = co_await kdCoroSignal<QString>(inputDialog, &InputDialog::filePath, this);

        if (!filepath.has_value()) {
            qDebug() << "Input dialog did not provide a finepath" << filepath.error();
            co_return;
        }

        QFile file(filepath);
        // save information

        MyDialog::accepted();
    };

    coroutine(); // co_await is forbidden
}
```

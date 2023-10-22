# EventFilter

A header only class to quickly "connect" an event to a callback.

It's common that classes sometimes miss signals and we're forced to write boilerplate event filters.
For example, QWidget::isEnabled() doesn't emit any signal when enabled changed.

You can now simply write:

```cpp
auto filter = KDToolBox::installEventFilter(button, QEvent::EnabledChange, [button] {
    qDebug() << "Enabled changed" << button->isEnabled();
    return false;
});
```

Two signatures are provided, one receiving a QObject parent as 4th argument and another one which
instead returns a std::unique_ptr.

# MessageHandler

Some convenience code to debug messages logged through Qt's logging facilities
(qDebug(), qWarning(), etc.).

You can use this code to figure out under what conditions a certain warning is
printed, for instance like this:

```cpp
KDToolBox::handleMessage(QtWarningMsg,
                         QRegularExpression("SomeClass::someFunction: .*"),
                         [](){ /* put a breakpoint here */ });
```

This avoids the annoyance of installing a global message handler just for
small debugging tasks.

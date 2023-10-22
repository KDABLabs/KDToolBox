# Qml Stacktrace Helper

An helper function so you can get a QML backtrace from gdb.

## Example Usage

Build `QmlStackTraceHelper.cpp` with your project.

```text
(gdb) call KDAB::printQmlStackTraces()
Stack trace for QQuickView(0x7fffffffd2e0 active exposed, visibility=QWindow::Windowed, flags=QFlags<Qt::WindowType>(Window), geometry=0,1290 500x500)
    onSomeIndirection2Changed [qrc:/main.qml:28]
    onSomeIndirectionChanged [qrc:/main.qml:22]
    onClicked [qrc:/main.qml:17]
```

Also useful when running under `valgrind --vgdb=yes`.
Just link `QmlStackTraceHelper.cpp` with your application.

## Upstreaming to Qt

We're open to ideas on how to upstream this.

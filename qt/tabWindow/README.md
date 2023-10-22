# TabWindow

The `TabWindow` widget is a specific widget working like a tab widget, but where tabs could be
extracted and become a new window, or be re-added later on to the main application. This is the
kind of behavior you see in modern browsers.

Note that behind the scene, the `TabWindow` is using a singleton manager, so it's not possible to
have multiple `TabWindow` in the same application.

## Example Usage

This widget is working like the `QTabWidget`, here is a small example:

```cpp
TabWindow w;
w.addTab(new QWidget, "Tab 1");
w.addTab(new QWidget, "Tab 2");
w.addTab(new QWidget, "Tab 3");
w.addTab(new QWidget, "Tab 4");
```

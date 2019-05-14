UI Watchdog
===========

Header-only tool that monitors the main thread and breaks the program whenever
its event loop hasn't run for `MAX_TIME_BLOCKED` (default 300ms).

Currently it only breaks the program when running on Windows, butt action to perform
can be costumized next to the "Add custom action here" comment.


Example Usage
==============


```
    #include "uiwatchdog.h"

    (...)

    UiWatchdog dog;
    dog.start();

    return app.exec();
```

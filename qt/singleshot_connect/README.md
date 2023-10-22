# singleshot-connect

An helper for QObject::connect() that connects a signal to a slot (or to a function object),
but creates a single shot connection: the connection gets automatically broken after
the first activation.

```cpp
KDToolBox::connectSingleShot(sender, &Sender::signal, receiver, &Receiver::slot);

sender->causeSignalEmission(); // calls the slot, and breaks the connection
sender->causeSignalEmission(); // does NOT call the slot

```

Requires a C++14 capable compiler.

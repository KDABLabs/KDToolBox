# NotifyGuard

This class is a generic property notification guard, that can
monitor if a property was modified between the point of construction
and destruction of the guard object. If the property was modified,
the property's notifySignal will be emitted on destruction of
the guard object. This is useful for classes with very complex
property settings, classes with interdependent properties and classes
where a single method may set change multiple properties at the
same time.

NotifyGuard requires the property to have a notify signal, and
that the notify signal has either no or one argument that must
then match the type of the property itself.

A NotifyGuard can be constructed using a property name or using
a pointer to a notify signal. In the first case, only the
indicated property will be monitored for changes. When using a
pointer to a notify signal, all properties using that notification
signal will be monitored for changes.

```cpp
NotifyGuard singlePropertyGuard(this, "thePropertyName");
NotifyGuard potentiallyMultiPropertyGuard(this, &ThisClass::theNotifySignal);
```

NotifyGuard can work either in SingleScope or in RecursiveScope
mode (the default). In SingleScope, NotifyGuard only cares about
itself and will always emit the notify signal if the value of the
property has changed on destruction of the guard object. In
RecursiveScope mode, NotifyGuard will make sure that a notification
signal is only emitted once, from the outermost scope created for
the given notify signal. This minimizes the number of signal
emissions to prevent potentially expensive updates.

```cpp
NotifyGuard singlePropertyGuard(this, "thePropertyName", NotifyGuard::SingleScope);
NotifyGuard potentiallyMultiPropertyGuard(this, &ThisClass::theNotifySignal, NotifyGuard::RecursiveScope);
```

RecursiveScope is the default mode and may be left off.

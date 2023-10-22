# KDSignalThrottler

This project contains an implementation of throttlers and debouncers
for signals and slots; that is, objects that rate-limit the activation
of a given slot.

* A **throttler** calls the slot only once every X ms, no matter what's
  the input frequency of the signal. Use case: filtering out input
  events which may occur at a very high frequency (example: mouse events);
  an expensive operation triggered by these events (like scrolling and
  repainting) can be performed less often.

* A **debouncer** activates the slot only once, after a timeout / grace
  period calculated since the last signal emission. In other words: if
  a signal keeps coming, the slot is _not_ activated. Use case: a
  search box that actually starts searching only after the user
  stops typing (that is, after a short timeout since the last user input).

Throttlers and debouncers can be trailing (the default) or leading.

* Trailing means that the throttler/debouncer does not activate the slot
  immediately, but waits until its own timeout occurs before activating.

* Leading means to activate the slot as soon as the throttler/debouncer
  itself is activated, and then does not trigger again until the timeout
  occurs (and only if another signal is received in the meanwhile).

The usage is straightforward. First and foremost create an instance
of the right debouncer/throttler class, and configure its timeout:

```cpp
// trailing throttler, rate limiting at 1 emission every 100ms
KDSignalThrottler *throttler = new KDSignalThrottler(parent);
throttler->setTimeout(100ms);
```

Connect the signal you want to rate limit to the `throttle` slot
of the throttler, and connect the `triggered` signal to the actual slot:

```cpp
connect(sender, &Sender::signal,
        throttler, &KDSignalThrottler::throttle);
connect(throttler, &KDSignalThrottler::triggered,
        receiver, &Receiver::slot);
```

Look into `KDSignalThrottler.h` for the other available types.

Requires a C++14 capable compiler.

Welcome to the Future
=====================

This is a collection of C++ standard library components backported to
C++11. They are designed to be drop-in replacements for the
corresponding `std` library components, so the only reference here is
the standard itself. Nothing must be added, though some features can
be left out, as long as those features which are implemented work
exactly as the `std` component.

Example: `k17::optional`

Ok difference:

- leave out `value_or()`

Not ok:

- make `value()` return by value
- not have a trivial copy ctor for trivially-copyable payloads

If you can, it's probably better to use corresponding Boost
components, because this library will never be complete.

When you use these components, though, we've made sure that you can
easily migrate to the corresponding `std` components if you increase
your project's minimum-required C++ version. E.g., if you update from
C++11 to C++17, then you can textually replace

   s,k14::,std::,g;
   s,<k14/([^.])\.h>,<\1>,

   s,k17::,std::,g;
   s,<k17/([^.])\.h>,<\1>,

Due to the way the header names and component namespaces are
constructed, this replacement won't change the layout of the source
code, preserving as much of the VCS history as possible.

Existing Components
-------------------

- `k20::erase`: C++20 Uniform Container Erasure

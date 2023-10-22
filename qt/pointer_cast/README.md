# pointer-cast

A collection of helper functions to ease porting from `QSharedPointer`
to `std::shared_ptr`, by helping with the most egregious API
incompatibilites of QSharedPointer, namely:

- static_pointer_cast overload for QSharedPointer
- dynamic_pointer_cast overload for QSharedPointer
- const_pointer_cast overload for QSharedPointer
- make_qshared - like std::make_shared, but returning QSharedPointer

The casts are designed to be used like their `std` counterparts
operating on `std::shared_ptr`. We advise to used unqualified lookup

```cpp
    QSharedPointer<Base> base = make_qshared<Derived>(~~~);
    auto derived = dynamic_pointer_cast<Derived>(base); // not KDToolBox::...
```

If you target C++20, then this is also the way to call
`std::dynamic_pointer_cast` (unqualifed lookup of functions with
explicit template arguments doesn't find candidates by ADL prior to
C++20) For this reason, the functions are defined in `inline namespace
KDToolBox`.

If you restrict your code to the std-compatible subset of
QSharedPointer and use the above four functions, then porting from
QSharedPointer to std::shared_ptr will be as easy as

```text
    s/QSharedPointer/std::shared_ptr/
    s/QWeakPointer/std::weak_ptr/
    s/make_qshared/std::make_shared/
```

Enjoy!

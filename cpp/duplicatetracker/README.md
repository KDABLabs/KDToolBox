# DuplicateTracker

DuplicateTracker is a helper class to keep track of "seen" elements, e.g.
to avoid handling duplicate elements:

```cpp
    DuplicateTracker<QString> tracker;

    for (auto &e : collection) {
        if (tracker.hasSeen(e))
            continue;
        peruse(e);
    }
```

DuplicateTracker transparently wraps `std::pmr::unordered_set` with a
`std::pmr::monotonic_memory_resource` in a non-copyable, non-movable
class with a narrowly-focussed API (basically, `hasSeen()`). If C++17
`<memory_resource>` isn't available, it falls back to the default
`std::allocator`.

Porting from, say, `QSet` to `DuplicateTracker` is simple:

```cpp
    -    QSet<QString> seen;
    -    seen.reserve(expectedNumElements);
    +    DuplicateTracker<QString> tracker(expectedNumElements);
    ~~~
    -    if (!seen.contains(x)) {
    -        seen.insert(x);
    -        peruse(x);
    -    }
    +    if (!tracker.hasSeen(x))
    +        peruse(x);
```

The second template argument (a size) instructs DuplicateTracker to size its
internal static buffer such that it can hold that many elements without
allocating memory from the heap. The tracker can contain more than these elements,
but it will allocate more memory from the heap to hold them (which will still be
much faster than using raw `QSet` or `std::unordered_set`).

If you use `DuplicateTracker` with Qt types that don't have `std::hash`-support,
yet, you want to use [Qt Hasher](https://github.com/KDAB/KDToolBox/tree/master/qt/qt_hasher)
as the third template argument:

```cpp
    DuplicateTracker<QModelIndex, 16, QtHasher<QModelIndex>> tracker(expectedNumElements);
```

Do _not_ specialize `std::hash` for types you don't control (that includes Qt types).
`QtHasher` works with Qt 5.14 and 5.15. If you specialized `std::hash` instead, it would
break in Qt 5.15, which added `std::hash<QString>`.

kdToContainer
=============

**A minimal reimplementation of `ranges::to` from ranges-v3.**

Qt 5.15 deprecated (and then Qt 6 removed) most of the converting
`toContainer()` functions on Qt containers (e.g. `QList<T>::toSet()`,
converting to a `QSet<T>`, has been deprecated).

The target type gained a range constructor instead. Now, while using
such constructor is indeed suggested as the *direct* replacement of
calling the conversion function, it's more verbose to use, and
requires an extra line in case of the source container is returned from
a function or so.

`kdToContainer` tries to bring back the convenience of the conversion
function, while being more general, as it works between any range-like
type (as the source) and any container-like type (as the destination):

```cpp
using namespace KDToolBox::Ranges;

QList<int> myList = ~~~;

// pipeline style
auto set1 = myList | kdToContainer<QSet>();      // converts the QList<int> to QSet<int>
auto set2 = myList | kdToContainer<QSet<int>>(); // ditto

// function call style
auto set3 = kdToContainer<QSet>(myList);         // ditto
auto set4 = kdToContainer<QSet<int>>(myList);    // ditto


auto vec = set1 | kdToContainer<QVector>(); // converts the QSet<int> to QVector<int>


QStringList getNames();
auto nameset = getNames() | kdToContainer<std::unordered_set>(); // converts to std::unordered_set<QString>

```

---

Note that the underlying reason for deprecating such functionality in
Qt containers is not merely because there's an equivalent, more general
one (so the "redundant", "limited" approach got removed).

*The real reason* is because most of the time conversions between
containers are hiding a design problem: usage of containers instead of
algorithms, poorly designed interfaces, and similar.

Therefore: before reaching for a conversion between containers, try to
understand if your _design_ is sound! For instance, if you want to remove
duplicates from a sequential container, use `std::sort` + `std::unique`;
if you want to process elements avoiding duplicate elements (without
altering the source container), then use
[DuplicateTracker](https://github.com/KDAB/KDToolBox/tree/master/cpp/duplicatetracker);
and so on.

**It's highly recommended to use ranges-v3 instead of this.**
The implementation offered here is knowingly incomplete in some cases.
Use this only if ranges-v3 doesn't work for you.

Requires a C++14 capable compiler.

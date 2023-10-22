KDStlContainerAdaptor
=====================

**Adaptors to ease the transition from Qt containers to the C++ STL containers.**

Qt containers feature a somehow richer and easy-to-use API than the STL
counterparts. For instance, this is possible on a QVector:

```cpp
QVector<int> v;

v << 1 << 2 << 3;     // chained push_back via operator<<
if (v.contains(42))   // algorithm as member
    doSomething();

qDebug() << v.first() << v.last(); // front/back
v.append(100); // push_back
v.prepend(0);  // "push_front"
v.removeAt(2); // removes at index #2
```

If, for whatever reason, we wish to migrate `v` from `QVector` to the
equivalent C++ Standard Library container (that is, to `std::vector`)
then we would need to fix all of these usages, making the porting quite
tedious.

`KDStlContainerAdaptor` tries to minimize the effort by letting the
existing source code compile mostly unchanged. All we need to do is to
to use its datatypes in place of of the Qt ones; the existing Qt-based
source code will compile unchanged.

To ease porting from `QVector` (and possibly `QList`) just use the
`StdVectorAdaptor` class template. It transparently inherits from
`std::vector`, but adds back the Qt-specific APIs:

```cpp
// Use this instead of QVector<int>
using IntVector = KDToolBox::StlContainerAdaptor::StdVectorAdaptor<int>;
IntVector v;  // is-a std::vector

// The previous code still works!
v << 1 << 2 << 3;     // chained push_back via operator<<
if (v.contains(42))   // algorithm as member
    doSomething();

qDebug() << v.first() << v.last(); // front/back
v.append(100); // push_back
v.removeAt(2); // removes at index #2
```

More helpers may come in the future.

Requires a C++17 capable compiler.

# QtHasher

A hasher to be able to use Qt datatypes in the unordered associative containers from
the Standard Library.

```cpp
std::unordered_map<QtType, Foo, KDToolBox::QtHasher<QtType>> map;

map.insert(~~~);
```

By default, types such as `std::unordered_map` or `std::unordered_set` require
a specialization of `std::hash` for the key type. The problem is that such a
specialization is not actually provided by most Qt datatypes -- only for string-like
and byte arrays (QString, QStringView, QByteArray, etc.), and only starting
from Qt 5.14.

On the other hand, most Qt value types do have a hashing function -- `qHash()` --
already defined for them. We can use that one to implement a custom hasher.

Note that one is not authorized to specialize customization points for types
not under their direct control. Case in point, one is not authorized to specialize
`std::hash` for Qt datatypes: Qt reserves the right to add specializations at
any time (breaking our code and violating ODR). Hence, we define a custom
hasher.

Requires a C++11 capable compiler.

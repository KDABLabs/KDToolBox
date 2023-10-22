# propagate_const

**
A minimal reimplementation of `std::experimental_::propagate_const` from
the C++ Extensions for Library Fundamentals version 3.
**

The propagate_const class is a deep-const wrapper for pointers and
smart pointers. If you have a class and want to ensure const
correctness even for its data members that are held by (smart)
pointer, wrap them in a propagate_const:

```cpp
class MyClass {
    KDToolBox::propagate_const<Data *> m_data;

public:
    void observer() const {
      m_data->mutate(); // ERROR
    }
};
```

"Ordinary" pointers (raw and smart) do not deeply propagate const, and
would let the user accidentally (or deliberately) mutate m_data above.
If the state of m_data logically belongs to the observable state of
`*this`, then the ordinary (smart) pointer would violate const
correctness. The usage of `propagate_const` prevents that mistake from
happening.

Requires a C++17 capable compiler.

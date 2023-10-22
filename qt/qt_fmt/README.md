# libfmt support for Qt datatypes

This project provides a convenience `fmt::formatter` partial
specialization which enables formatting of Qt-related datatypes using
[libfmt](https://github.com/fmtlib/fmt).

The formatting is implemented by reusing the QDebug streaming
operators. Most Qt datatypes already offer such an operator:

```cpp
    QDateTime dt = ~~~;
    qDebug() << dt; // QDateTime can be streamed into QDebug
```

This header-only project allows to format/print such an object using
fmt's facilities, for instance like this:

```cpp
    #include <qt_fmt.h>

    QDateTime dt = ~~~;
    fmt::print("{}\n", dt); // works
```

No other code is necessary.

This approach works with *any* datatype that has a QDebug streaming
operator, whether it's a Qt-provided class or a user-defined one:

```cpp
    // A custom class that has a QDebug operator
    class MyClass { ~~~ };
    QDebug operator<<(QDebug stream, const MyClass &c) { ~~~ }

    void f() {
        MyClass obj;
        fmt::print("My object is: {}\n", obj); // works
    }
```

Requires a C++17-capable compiler.

Note: unlike `libfmt`'s facilities, `QDebug` isn't designed with
efficiency in mind. The formatter can help integrating `libfmt` within
Qt-based projects, but it should not be used in scenarios where
performances are paramount.

## Formatting options

At the moment, only `{}` is supported.

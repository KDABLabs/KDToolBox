# Qml PropertySelector

A QML item to help set properties based on conditions

PropertySelector allows you to select which values to set on one or more properties based
on a set of other boolean properties. This enables you to easily express complex
conditions, for instance have the color of an item differ based on selection, hover, checked and
enabled status without complex binding expressions or imperative code.

PropertySelector is a class that employs custom parsing of its contents, allowing it to
use the QML syntax in a slightly different way. PropertySelector operates on a *target*
which by default is the parent item. That is the only conventional property that it exposes.

## Usage

### Rules, Conditions, Target Properties and Values

PropertySelector can be used by specifying Rules. A Rule is a construct that specifies 0 or
more Conditions, a Target Property and a (Value) Binding for that property. Both Conditions
and Target Properties must be properties in the @ref target item. The Condition properties
must be boolean properties that have a notify signal, while the Target Properties must be
writable but may otherwise be of any type.

The PropertySelector binds to all Condition properties specified in the rules, and sets the
target properties to the values specified in the most specific matching rule.

To specify rules, PropertySelector employs the syntax for grouped properties. Each rule
consists of zero or more Conditions that take the place of the (nested) property groups,
with the Target Property taking the place of the (grouped) property. The Value Binding is
the value that should be set if the rule matches the actual conditions on the target item.

For example:

```qml
PropertySelector {
    checked.color: "red"
    hovered.color: _style.hoveredColor
    checked.hovered.color: _style.checkedHoveredColor
    color: "#dddddd"
}
```

The nested syntax for grouped properties is available too, so you can also use:

```qml
PropertySelector {
    checked {
        color: "red"
        hovered.color: _style.checkedHoveredColor
    }

    hovered.color: _style.hoveredColor
    color: "#dddddd"
}
```

### Order of evaluation

In the examples above, we have four different Rules all specifying the same Target Property
`color` for different Conditions. The rules to determine which Rule is used when are as
follows: For any Target Property, the rule with the highest number of properties that are
all fulfilled is applied (so, the most specific one). If there are more rules that have the
same number of fulfilled properties, the rule specified last will be applied.

In the example above, the rule without any conditions specifying a Value Binding `#dddddd`
will only be applied if neither the `checked` nor the `hovered` conditions are fulfilled.
If both are fulfilled, the Rule specifying both will be applied as this is the most
specific rule that matches the Conditions. If only the `checked` Condition is fulfilled,
the `red` Value Binding would be applied.

### Use cases

This Item may be used to simplify the specification of complex conditions for properties,
which would otherwise require complex cascades of tenairy conditions in a binding or
imperative code.

### Workings and limitations

This Item works by writing values the values from the Value Bindings to the Target
Properties. Expressions for these values are supported, but currently don't result in
actual property bindings. Instead, they are evaluated at the moment the assignment is
made. Using objects or translations is currently *not* supported. Note that if no rule
matches, the original value or property binding that was on the *target* item's
property is *not* restored. It is recommended to always have a condition-less Rule in
place for each Target property as the default value.

## Registration in C++

Add the property PropertySelector class to your project, and register the class like this:

```cpp
#include "PropertySelector.h"
static cpp::qmltypes::PropertySelectorCustomParser propertySelectorParser;
qmlRegisterCustomType<cpp::qmltypes::PropertySelector>("MyLibraryName", 1, 0, "PropertySelector", &propertySelectorParser);
```

You will need to link to QtQuick private headers, as this uses the private custom parser API.

## Upstreaming to Qt

We're open to ideas on how to improve and upstream this.

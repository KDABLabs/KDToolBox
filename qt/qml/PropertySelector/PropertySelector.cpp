/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "PropertySelector.h"
#include <QDebug>
#include <QQmlContext>
#include <QQmlProperty>

using namespace cpp::qmltypes;

/*!
  @class PropertySelector
  @brief A QML Item to set one or more properties on a target based on a set of rules

  PropertySelector allows you to select which values to set on one or more properties based
  on a set of other boolean properties. This enables you to easily express complex
  conditions, for instance to have the color of an item differ based on selection, hover,
  checked and enabled status without complex binding expressions or imperative code.

  PropertySelector is a class that employs custom parsing of its contents, allowing it to
  use the QML syntax in a slightly different way. PropertySelector operates on a @ref target
  which by default is the parent item. That is the only conventional property that it exposes.

  ##Rules, Conditions, Target Properties and Values##
  PropertySelector can be used by specifying Rules. A Rule is a construct that specifies 0 or
  more Conditions, a Target Property and a (Value) Binding for that property. Both Conditions
  and Target Properties are properties in the @ref target item. The Condition properties
  must be boolean properties that have a notify signal, while the Target Properties must be
  writable but may otherwise be of any type.

  To specify rules, PropertySelector employs the syntax for grouped properties. Each rule
  consists of zero or more Conditions that take the place of the (nested) property groups,
  with the Target Property taking the place of the (grouped) property. The Value Binding is
  the value that should be set if the rule matches the actual conditions on the target item.

  For example:
  \code{.qml}
  PropertySelector {
    checked.color: "red"
    hovered.color: _style.hoveredColor
    checked.hovered.color: _style.checkedHoveredColor
    color: "#dddddd"
  }
  \endcode

  The nested syntax for grouped properties is available too, so you can also use:
  \code{.qml}
  PropertySelector {
    checked {
      color: "red"
      hovered.color: _style.checkedHoveredColor
    }

    hovered.color: _style.hoveredColor
    color: "#dddddd"
  }
  \endcode

  ##Order of evaluation##
  In the examples above, we have four different Rules all specifying the same Target Property
  `color` for different Conditions. The rules to determine which Rule is used when are as
  follows: For any Target Property, the rule with the highest number of properties that are
  all fulfilled is applied. If there are more rules that have the same number of fulfilled
  properties, the rule specified last will be applied.

  In the example above, the rule without any conditions specifying a Value Binding `#dddddd`
  will only be applied if neither the `checked` nor the `hovered` conditions are fulfilled.
  If both are fulfilled, the Rule specifying both will be applied as this is the most
  specific rule that matches the Conditions. If only the `checked` Condition is fulfilled,
  the `red` Value Binding would be applied.

  ##Use cases##
  This Item may be used to simplify the specification of complex conditions for properties,
  which would otherwise require complex cascades of tenairy conditions in a binding or
  imperative code.

  ##Workings and limitations##
  This Item works by writing values the values from the Value Bindings to the Target
  Properties. Expressions for these values are supported, but currently don't result in
  actual property bindings. Instead, they are evaluated at the moment the assignment is
  made. Using objects or translations is currently *not* supported. Note that if no rule
  matches, the original value or property binding that was on the @ref target item's
  property is *not* restored. It is recommended to always have a condition-less Rule in
  place for each property as the default value.
  */

///@brief constructor
///@param parent the parent item
PropertySelector::PropertySelector(QQuickItem *parent)
    : QObject(parent)
{
}

///@brief Getter for the target.
/// If no target has been set explicitly, the target is the parent item.
QObject *PropertySelector::target() const
{
    if (m_target)
        return m_target;
    return parent();
}

///@brief Setter for the target.
void PropertySelector::setTarget(QObject *target)
{
    if (target != m_target)
    {
        m_target = target;
        emit targetChanged(target);
        connectChangeSignals();
        apply();
    }
}

///@brief reimplemented from QQmlParserStatus for internal reasons
///@internal
void PropertySelector::componentComplete()
{
    if (!m_ruleList.isEmpty())
    {
        prepareRules();
        connectChangeSignals();
        apply();
    }
}

///@brief reimplemented from QQmlParserStatus for internal reasons
///@internal
void PropertySelector::classBegin() {}

///@brief connects-up the change signals from the targets condition properties to our internal handling method
///@internal
void PropertySelector::connectChangeSignals()
{
    if (!target())
        return;

    for (const QString condition : m_conditions)
    {
        QQmlProperty prop(target(), condition);
        if (prop.isValid() && prop.hasNotifySignal() && prop.propertyType() == QMetaType::Bool)
        {
            static int applyIndex = metaObject()->indexOfMethod("apply()");
            prop.connectNotifySignal(this, applyIndex);
        }
        else
        {
            qWarning() << "PropertySelector: specified target does not have (notifiable) boolean property" << condition;
        }
    }
}

///@brief applies the list of rules based on the current status of the condition properties in @ref target()
///@internal
/// This method is triggered whenever any of the condition properties change
void PropertySelector::apply()
{
    // TODO: optimize so that only properties depending on the triggering condition get their rules re-evaluated?

    // build up a bit-field representation of the conditions that are enabled.
    ConditionBits currentBits;
    for (int i = 0; i < m_conditions.count(); ++i)
    {
        // set the bits corresponding to the fulfilled properties
        QQmlProperty prop(target(), m_conditions[i]);
        currentBits.set(i, prop.read().toBool());
    }

    //    qDebug() << "\ncurrent bits" << currentBits.to_string().c_str();

    // iterate over properties
    for (auto &propertyRules : m_properties)
    {
        // search for the first rule where all conditions are fulfilled
        //        qDebug() << "applying rules for" << propertyRules.property.name();
        const auto it = std::find_if(propertyRules.begin, propertyRules.end, [currentBits](const Rule &rule) {
            // The rules conditions fulfill the current state if or-ed together all bits are set. rule.conditionBits is
            // a bitmask with the needed contions set to false
            //            qDebug() << "rule bits" << rule.conditionBits.to_string().c_str() << " Or-ed:" <<
            //            (rule.conditionBits | currentBits).to_string().c_str() << rule.simpleValue;
            return (rule.conditionBits | currentBits).all();
        });
        if (it != propertyRules.end)
        {
            // found a rule to apply
            applyRule(propertyRules, it);
        }
    }
}

///@brief applies a specific rule
///@internal
/// @param propertyRules a reference to the PropertyRules instance. This provides direct access to the QQmlProperty to
/// manipulate
/// @param an iterator pointing to the rule that needs to be applied to the property. This provides access to the value
/// to set.
void PropertySelector::applyRule(PropertyRules &propertyRules, QVector<Rule>::const_iterator rule)
{
    // TODO: add support for script bindings
    if (!propertyRules.property.isValid())
        return;

    if (propertyRules.currentRule == rule)
        return;

    const auto currentProperty = propertyRules.property;
    delete propertyRules.currentBinding;

    if (rule->simpleValue.isValid())
    {
        //        qDebug() << "applying rule on property" << rule->property << "defined on line" << rule->location.line
        //        << "setting value" << rule->simpleValue;
        currentProperty.write(rule->simpleValue);
        propertyRules.currentBinding = nullptr;
    }
    else
    {
        QV4::Scope scope(qmlEngine(this)->handle());
        QQmlContextData *context = QQmlContextData::get(qmlContext(this));
        QV4::Scoped<QV4::QmlContext> qmlContext(
            scope, QV4::QmlContext::create(scope.engine->rootContext(), context, target()));
        QQmlBinding *binding =
            QQmlBinding::create(&QQmlPropertyPrivate::get(currentProperty)->core,
                                m_compilationUnit->runtimeFunctions.at(rule->id), target(), context, qmlContext);
        QVariant value = binding->evaluate();
        if (value.isValid())
        {
            //            qDebug() << "applying rule on property" << rule->property << "defined on line" <<
            //            rule->location.line << "setting value from script" << value;

            currentProperty.write(value);
        }
        else
        {
            qWarning() << "Evaluation of expression resulted in invalid value! Line:" << rule->location.line;
        }
        binding->setTarget(currentProperty);
        //        QQmlPropertyPrivate::setBinding(binding, QQmlPropertyPrivate::None,
        //        QQmlPropertyData::DontRemoveBinding | QQmlPropertyData::BypassInterceptor);
        //        propertyRules.currentBinding = binding;
        delete binding;
    }

    propertyRules.currentRule = rule;
}

///@brief prepares the rules set by the custom parser for use in the running component
///@internal
/// This method:
/// 1. builds up the list of conditions that need to be wached,
/// 2. sorts the rules by property and then in order they need to be applied in if the conditions match,
/// 3. builds up m_properties with pointers to the properties and pointers into the rules list
void PropertySelector::prepareRules()
{
    QStringList conditions;
    for (const Rule &rule : qAsConst(m_ruleList))
    {
        conditions.append(rule.conditions);
    }

    std::sort(conditions.begin(), conditions.end());
    conditions.erase(std::unique(conditions.begin(), conditions.end()), conditions.end());

    m_conditions = conditions;

    // Sort the rules. First, group by property, then sort by number of conditions, then by position in the source.
    // This way, per property, the rule that is hardest to fulfill will be first in the list
    std::sort(m_ruleList.begin(), m_ruleList.end());

    // Map the string-based conditions to a bit set for faster comparisons
    for (Rule &rule : m_ruleList)
    {
        rule.conditionBits = bitmaskFromConditionList(rule.conditions);
        //        qDebug() << rule.property << "Rule bits for state" << rule.simpleValue <<
        //        rule.conditionBits.to_string().c_str();
    }

    m_properties.clear();
    const auto appendProperty = [&](QVector<Rule>::iterator start, QVector<Rule>::iterator end) {
        //        qDebug() << "adding range for property" << start->property << start - m_ruleList.begin() << "length"
        //        << end - start;
        if (start != end)
        {
            QQmlProperty prop(target(), start->property, QQmlEngine::contextForObject(this));
            if (!prop.isValid())
            {
                // format an error message
                auto context = QQmlEngine::contextForObject(target());
                const QString objectName = context->objectName().isEmpty() ? QStringLiteral("unnamed")
                                                                           : context->objectName(); // FIXME: broken
                auto switchContext = QQmlEngine::contextForObject(this);
                qWarning(
                    QString(
                        "PropertySelector target %1 (%2) does not have a property '%3' requested from %4, line %5:%6")
                        .arg(target()->metaObject()->className(), objectName, start->property,
                             switchContext->baseUrl().toString(), QString::number(start->location.line),
                             QString::number(start->location.column))
                        .toLatin1());
            }
            else
            {
                PropertyRules rules{prop, start, end, end, nullptr};
                m_properties.append(rules);
            }
        }
    };

    auto rangeStart = m_ruleList.begin();
    auto it = rangeStart; // std::next(rangeStart);
    const auto endIt = m_ruleList.end();

    // generate a list of properties with pointers into the rule list to the ranges of rules relating to this property
    do
    {
        it = std::find_if(rangeStart, endIt, [=](const Rule &rule) { return rangeStart->property != rule.property; });
        appendProperty(rangeStart, it);
        rangeStart = it;
    } while (rangeStart != endIt);
}

///@brief generates a bitmask based on the list of conditions monitored.
///@internal
/// The mask is a bitfield with the bits corresponding to the conditions that need to be fulfilled set to
/// false, and all other bits set to true. This mask, when or-ed with a bitfield representing the currently
/// fulfilled conditions, will result in a bitfield with all bits set to true, which is very quick to test.
PropertySelector::ConditionBits PropertySelector::bitmaskFromConditionList(const QStringList &conditions) const
{
    ConditionBits bits;
    int leftToFind = conditions.count(); // way to break out of the loop early if we found all conditions in our list
    for (int i = 0; i < m_conditions.count() && leftToFind > 0; ++i)
    {
        if (conditions.contains(m_conditions[i]))
        {
            bits.set(i);
            --leftToFind;
        }
    }
    return ~bits;
}

///@brief function that provides the opportunity for early verification of the bindings
///@internal
/// Currently not used.
void PropertySelectorCustomParser::verifyBindings(
    const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compiledUnit,
    const QList<const QV4::CompiledData::Binding *> &bindings)
{
    Q_UNUSED(compiledUnit);
    Q_UNUSED(bindings);
}

///@brief function that is called by the QML engine to interpret the non-recognized bindings
///@internal
///@param object the PropertySelector instance the parser is operating on
///@param compiledUnit a QML-engine data structure that contains data on the JS compilation unit
///@param bindings list with QML-engine binding objects to unpack
/// This is the function that is used to iterate over the rules. It calls the actual parser @ref parseBinding for each
/// rule it encounters.
void PropertySelectorCustomParser::applyBindings(QObject *object,
                                                 const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compiledUnit,
                                                 const QList<const QV4::CompiledData::Binding *> &bindings)
{
    auto propSwitch = qobject_cast<PropertySelector *>(object);
    Q_ASSERT(propSwitch);

    propSwitch->m_compilationUnit = compiledUnit;
    foreach (const QV4::CompiledData::Binding *binding, bindings)
    {
        parseBinding(propSwitch, {}, binding);
    }
}

///@brief function to parse an individual binding
///@internal
///@param selector the PropertySelector instance the parser is operating on
///@param conditions the current list of conditions already parsed. Each condition will trigger a new call to
/// parseBinding.
///@param binding QML-engine binding objects to unpack
/// This is the function that is used to actually parse a single binding. A binding is a unit of compilation in the QML
/// engine. When using grouped properties like we do for conditions, a binding is a single level of the group, or the
/// actual property with its value binding. If the binding represents a grouped property, we recursively unpack it by
/// adding the current item to the list of conditions, and calling parseBinding again on each of the sub-bindings
/// inside. If a binding represents a binding to a simple value, the value is read into a QVariant which is stored with
/// the property in the list of rules of the PropertySelector. If the binding represents a more complex expression, the
/// actual binding is stored in the rule list of the PropertySelector together with the name of the property to set and
/// the location of binding. The PropertySelector will then when needed evaluate the binding to a value to set on the
/// indicated property of the target object.
void PropertySelectorCustomParser::parseBinding(PropertySelector *selector, QStringList conditions,
                                                const QV4::CompiledData::Binding *binding)
{
    QVariant simpleValue;

    QString name = selector->m_compilationUnit->stringAt(binding->propertyNameIndex);

    switch (binding->type)
    {
    case QV4::CompiledData::Binding::Type_Boolean:
        simpleValue = binding->valueAsBoolean();
        break;
    case QV4::CompiledData::Binding::Type_Number:
        simpleValue = binding->valueAsNumber(selector->m_compilationUnit->constants);
        break;
    case QV4::CompiledData::Binding::Type_Script:
    {
        bool ok;
        QByteArray script = selector->m_compilationUnit->stringAt(binding->stringIndex).toUtf8();
        int enumValue = evaluateEnum(script, &ok);
        if (ok)
        {
            // we have an enum value
            simpleValue = enumValue;
            break;
        }
        else
        {
            selector->m_ruleList.append({std::move(conditions), std::move(name), binding, binding->location});
            return;
        }
    }
    case QV4::CompiledData::Binding::Type_String:
        simpleValue = binding->valueAsString(selector->m_compilationUnit.data());
        break;
    case QV4::CompiledData::Binding::Type_GroupProperty:
        if (conditions.contains(name))
            error(binding, QString("Repeating condition `%1`").arg(name));

        conditions.append(name);
        {
            const QV4::CompiledData::Object *subObj = selector->m_compilationUnit->objectAt(binding->value.objectIndex);
            const QV4::CompiledData::Binding *subBinding = subObj->bindingTable();

            for (quint32 i = 0; i < subObj->nBindings; ++i, ++subBinding)
            {
                parseBinding(selector, conditions, subBinding);
            }
        }
        return;
    case QV4::CompiledData::Binding::Type_Translation:
    case QV4::CompiledData::Binding::Type_TranslationById:
    default:
        error(binding, QString("This binding type is not supported by PropertySelector. Only use simple values, enums "
                               "or property bindings."));
        return;
    }

    selector->m_ruleList.push_back({std::move(conditions), std::move(name), simpleValue, binding->location});
}

///@brief Compare operator on Rule struct to make them sortable
///@internal
///@param other the rule instance to compare with
/// Returns an inverse order based on first the property name, then the number of conditions, then
/// the location it was defined in.
bool PropertySelector::Rule::operator<(const PropertySelector::Rule &other)
{
    const auto thisTuple = std::make_tuple(property, conditions.count(), location);
    const auto otherTuple = std::make_tuple(other.property, other.conditions.count(), other.location);
    return thisTuple > otherTuple; // we want a reverse order
}

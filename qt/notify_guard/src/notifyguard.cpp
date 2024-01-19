/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "notifyguard.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QVarLengthArray>
#include <unordered_map>
#include <vector>

static const char guardSharedDataPropertyName[] = "__NotifyGuardSharedData__";
namespace
{
Q_DECLARE_LOGGING_CATEGORY(cat)
Q_LOGGING_CATEGORY(cat, "KDToolBox.NotifyGuard", QtWarningMsg);
}

namespace KDToolBox
{
namespace Internal
{

struct PropertyPair
{
    QMetaProperty property;
    QVariant startValue;
};
// the per-signal data. A shared instance is held by each NotifyGuard on this signal
struct SignalData
{
    QObject *target;
    QMetaMethod signal;
    QVarLengthArray<PropertyPair, 2> properties;

    SignalData() = default;
    ~SignalData();
    Q_DISABLE_COPY(SignalData)
    bool contains(const QMetaProperty &property) const;
};
using SignalDataWPtr = std::weak_ptr<SignalData>;
struct SignalIdDataPair
{
    int id;
    SignalDataWPtr dataWPtr;
};

using InstanceData = QVarLengthArray<SignalIdDataPair, 4>;
using InstanceDataPtr = std::shared_ptr<InstanceData>;
}

static Internal::SignalDataSPtr getDataObject(QObject *target, QMetaMethod notifySignal,
                                              NotifyGuard::GuardOptions options);

/**
 * @brief NotifyGuard::NotifyGuard constructs a property guard on the given target and property
 * @param target the object to monitor. Usually `this`
 * @param property the property to monitor, given as a string name
 * @param options use only in this scope or consider other guards on the same property in other scopes as well.
 *
 * By default, NotifyGuard uses RecursiveScope for @arg options.
 */
NotifyGuard::NotifyGuard(QObject *target, const char *property, GuardOptions options)
{
    Q_ASSERT(target);
    qCDebug(cat) << "Creating NotifyGuard from property on" << property;

    auto metaObject = target->metaObject();
    int propertyIndex = metaObject->indexOfProperty(property);

    if (propertyIndex < 0)
    {
        qCWarning(cat) << "Error: Constructing NotifyGuard on non-existent property" << property << "on target"
                       << target;
        return;
    }

    const QMetaProperty prop = metaObject->property(propertyIndex);
    auto signal = prop.notifySignal();
    if (!signal.isValid())
    {
        qCWarning(cat) << "Error: Constructing NotifyGuard on property `" << property << "` on target" << target
                       << "that does not have a notify signal";
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!prop.metaType().isEqualityComparable())
#else
    if (prop.type() == QVariant::UserType && !QMetaType::hasRegisteredComparators(prop.userType()))
#endif
    {
        qCWarning(cat) << "Error: Constructing NotifyGuard on property" << prop.name()
                       << "which is of a user-defined type" << prop.typeName()
                       << "that does not have a comparison operator registered. "
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) // no need for explicit registration any more in Qt6
                          "Register a comparison operator for this type using QMetaType::registerEqualsComparator<"
                       << prop.typeName() << ">()."
#endif
            ;
        return;
    }

    const auto parameterCount = signal.parameterCount();
    if (parameterCount > 1)
    {
        qCWarning(cat) << "Error: Constructing NotifyGuard on property `" << property << "` on target" << target
                       << "with a notify signal with too many parameters";
        return;
    }
    else if (parameterCount == 1)
    {
        if (signal.parameterType(0) != prop.userType())
        {
            qCWarning(cat) << "Error: Constructing NotifyGuard on property `" << property << "` on target" << target
                           << "with a notify signal that has a different parameter type than the property itself";
            return;
        }
    }

    m_signalData = getDataObject(target, signal, options);

    // check if we already monitor this property
    if (!m_signalData->contains(prop))
    {
        m_signalData->properties.push_back({prop, prop.read(target)});
    }
}

NotifyGuard::~NotifyGuard()
{
    qCDebug(cat) << "Destroying NotifyGuard. signalData object" << m_signalData.get();
}

/**
 * @fn NotifyGuard::isActive() const
 * @brief NotifyGuard::isActive indicates whether the NotifyGuard is active
 *
 * A NotifyGuard is active if it is actively monitoring one or more properties.
 * Normally, a NotifyGuard would be active. It can be inactive if it was created
 * on an invalid property or signal.
 *
 * @returns true if the NotifyGuard is active, false if not.
 */

/**
 * @brief NotifyGuard::NotifyGuard constructs a property guard on the given target and property
 * @param target the object to monitor. Usually `this`
 * @param notifySignal The notifySignal for the property or properties to monitor.
 * @param options use only in this scope or consider other guards on the same property in other scopes as well.
 *
 * By default, NotifyGuard uses RecursiveScope for @arg options.
 */
NotifyGuard::NotifyGuard(QObject *target, QMetaMethod notifySignal, NotifyGuard::GuardOptions options)
{
    qCDebug(cat) << "Creating NotifyGuard from signal" << notifySignal.name() << "on object" << target;
    if (!notifySignal.isValid())
    {
        qCWarning(cat) << "Error: Constructing NotifyGuard on invalid signal on target" << target;
        return;
    }

    auto metaObject = target->metaObject();

    auto data = getDataObject(target, notifySignal, options);

    bool foundProperties = false;
    bool foundUncomparableProperties = false;
    for (int i = 0; i < metaObject->propertyCount(); ++i)
    {
        const auto property = metaObject->property(i);
        if (property.notifySignal() == notifySignal)
        {
            // check if the types are comparable
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (property.metaType().isEqualityComparable())
#else
            if (property.type() != QVariant::UserType || QMetaType::hasRegisteredComparators(property.userType()))
#endif
            {
                foundProperties = true;
                if (!data->contains(property))
                {
                    data->properties.push_back({property, property.read(target)});
                }
            }
            else
            {
                qCWarning(cat)
                    << "Warning: Constructing NotifyGuard on signal" << notifySignal.name()
                    << "that is used for property" << property.name() << "which is of a user-defined type"
                    << property.typeName()
                    << "that does not have a comparison operator registered. "
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) // no need for explicit registration any more in Qt6
                       "Register a comparison operator for this type using QMetaType::registerEqualsComparator<"
                    << property.typeName()
                    << ">(). "
#endif
                       "Property"
                    << property.name() << "will not be used.";
                foundUncomparableProperties = true;
            }
        }
    }

    if (foundProperties)
    {
        m_signalData = std::move(data);
    }
    else
    {
        if (foundUncomparableProperties)
        {
            qCWarning(cat) << "Error: Constructing NotifyGuard on signal" << notifySignal.name()
                           << "failed because all properties using this notification signal have "
                              "custom types that don't have a registered comparison operator.";
        }
        else
        {
            qCWarning(cat) << "Error: Constructing NotifyGuard on signal" << notifySignal.name()
                           << "failed as there are no properties using this signal as a notification signal.";
        }
    }
}

Internal::SignalDataSPtr getDataObject(QObject *target, QMetaMethod notifySignal, NotifyGuard::GuardOptions options)
{
    auto createSignalData = [](QObject *target, QMetaMethod notifySignal) {
        Internal::SignalDataSPtr shared = std::make_shared<Internal::SignalData>();
        shared->target = target;
        shared->signal = notifySignal;
        return shared;
    };

    if (options == NotifyGuard::RecursiveScope)
    {
        const auto notifySignalIndex = notifySignal.methodIndex();

        qCDebug(cat) << "custom properties on target" << target << target->dynamicPropertyNames()
                     << "looking for signal id" << notifySignalIndex;

        auto instanceData = target->property(guardSharedDataPropertyName).value<Internal::InstanceDataPtr>();
        if (instanceData)
        {
            qCDebug(cat) << "instance data ptr found and valid";
            const auto it = std::find_if(
                instanceData->begin(), instanceData->end(),
                [notifySignalIndex](const Internal::SignalIdDataPair &data) { return notifySignalIndex == data.id; });
            if (it != instanceData->end())
            {
                if (auto sptr = it->dataWPtr.lock())
                {
                    qCDebug(cat) << "re-using shared instanceData for" << notifySignal.name() << sptr.get();
                    return sptr;
                }

                qCDebug(cat) << "found shared instance, but unable to lock it";

                // replace the current, no-longer valid entry with a new one
                auto shared = createSignalData(target, notifySignal);
                *it = {notifySignalIndex, Internal::SignalDataWPtr(shared)};
                qCDebug(cat) << "created new shared instanceData for" << notifySignal.name() << shared.get();

                return shared;
            }
        }
        else
        {
            qCDebug(cat) << "instance data ptr not found";
            instanceData = std::make_shared<Internal::InstanceData>();
            target->setProperty(guardSharedDataPropertyName, QVariant::fromValue(instanceData));
        }

        auto shared = createSignalData(target, notifySignal);
        instanceData->push_back({notifySignalIndex, Internal::SignalDataWPtr(shared)});
        qCDebug(cat) << "created new shared instanceData for" << notifySignal.name() << shared.get();
        return shared;
    }
    else
    {
        qCDebug(cat) << "created new non-shared instanceData for" << notifySignal.name();
        return createSignalData(target, notifySignal);
    }
}

/**
 * @fn NotifyGuard::~NotifyGuard
 * @brief NotifyGuard::~NotifyGuard destructor
 *
 * The destructor triggers the evaluation if the signal should be emitted and does the
 * emission of the signal if needed.
 */

/**
 * @brief NotifyGuard::SignalData::~SignalData
 * @internal
 * The destructor triggers the evaluation if the signal should be emitted and does the
 * emission of the signal if needed. The trick is that every instance of NotifyGuard
 * holds a shared pointer to an instance specific to the notify signal. That means that
 * the root guard object will be the one destroying the SignalData instance and thus
 * triggering the signal emissions.
 */
Internal::SignalData::~SignalData()
{
    qCDebug(cat) << "Destroying SignalData object" << this << "on signal" << signal.name();
    for (const auto &pp : properties)
    {
        const auto currentValue = pp.property.read(target);
        if (currentValue == pp.startValue)
            continue;
        // we need to emit the signal
        if (signal.parameterCount() == 0)
        {
            signal.invoke(target, Qt::DirectConnection);
        }
        else
        { // one parameter of the same type as the property itself
            signal.invoke(target, Qt::DirectConnection,
                          QGenericArgument(signal.parameterTypes().constFirst().data(), currentValue.data()));
        }
        qCDebug(cat) << "... emitted signal";
        break;
    }
}

bool Internal::SignalData::contains(const QMetaProperty &property) const
{
    const auto propertyIndex = property.propertyIndex();
    return std::any_of(properties.begin(), properties.end(), [propertyIndex](const PropertyPair &pp) {
        return pp.property.propertyIndex() == propertyIndex;
    });
}

} // end namespace KDToolBox

Q_DECLARE_METATYPE(KDToolBox::Internal::InstanceDataPtr);

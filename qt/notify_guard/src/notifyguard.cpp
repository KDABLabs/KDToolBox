/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: André Somers <andre.somers@kdab.com>
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, ** and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice (including the next paragraph)
** shall be included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
****************************************************************************/

#include "notifyguard.h"
#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include <QVarLengthArray>
#include <vector>
#include <unordered_map>

static const char guardSharedDataPropertyName[] = "__NotifyGuardSharedData__";

namespace KDToolBox {
namespace Internal {

struct PropertyPair {
    QMetaProperty property;
    QVariant startValue;
};
//the per-signal data. A shared instance is held by each NotifyGuard on this signal
struct SignalData {
    QObject* target;
    QMetaMethod signal;
    QVarLengthArray<PropertyPair, 2> properties;

    ~SignalData();
    bool contains(const QMetaProperty& property) const;
};
using SignalDataWPtr = std::weak_ptr<SignalData>;
struct SignalIdDataPair {
    int id;
    SignalDataWPtr dataWPtr;
};

using InstanceData = QVarLengthArray<SignalIdDataPair, 4>;
using InstanceDataPtr = std::shared_ptr<InstanceData>;
}


static Internal::SignalDataSPtr getDataObject(QObject* target, const QMetaMethod& notifySignal, NotifyGuard::GuardOptions options);

/**
 * @brief NotifyGuard::NotifyGuard constructs a property guard on the given target and property
 * @param target the object to monitor. Usually `this`
 * @param property the property to monitor, given as a string name
 * @param options use only in this scope or consider other guards on the same property in other scopes as well.
 *
 * By default, NotifyGuard uses RecursiveScope for @arg options.
 */
NotifyGuard::NotifyGuard(QObject* target, const char* property, GuardOptions options)
{
    Q_ASSERT(target);

    auto metaObject = target->metaObject();
    int propertyIndex = metaObject->indexOfProperty(property);

    if (propertyIndex < 0) {
        qWarning() << "Error: Constructing NotifyGuard on non-existent property" << property << "on target" << target;
        return;
    }

    const QMetaProperty prop = metaObject->property(propertyIndex);
    auto signal = prop.notifySignal();
    if (!signal.isValid()) {
        qWarning() << "Error: Constructing NotifyGuard on property `" << property << "` on target" << target
                   << "that does not have a notify signal";
        return;
    }

    //fixme: we need some way to determine if the type is comparable or not
    const auto parameterCount = signal.parameterCount();
    if (parameterCount > 1) {
        qWarning() << "Error: Constructing NotifyGuard on property `" << property << "` on target" << target
                   << "with a notify signal with too many parameters";
        return;
    } else if (parameterCount == 1) {
        if (signal.parameterType(0) != prop.userType()) {
            qWarning() << "Error: Constructing NotifyGuard on property `" << property << "` on target" << target
                       << "with a notify signal that has a different parameter type than the property itself";
            return;
        }
    }

    m_signalData = getDataObject(target, signal, options);

    // check if we already monitor this property
    if (!m_signalData->contains(prop)) {
        m_signalData->properties.push_back({prop, prop.read(target)});
    }
}

/**
 * @brief NotifyGuard::NotifyGuard constructs a property guard on the given target and property
 * @param target the object to monitor. Usually `this`
 * @param notifySignal The notifySignal for the property or properties to monitor.
 * @param options use only in this scope or consider other guards on the same property in other scopes as well.
 *
 * By default, NotifyGuard uses RecursiveScope for @arg options.
 */
NotifyGuard::NotifyGuard(QObject* target, const QMetaMethod& notifySignal, NotifyGuard::GuardOptions options)
{
    if (!notifySignal.isValid()) {
        qWarning() << "Error: Constructing NotifyGuard on invalid signal on target" << target;
        return;
    }

    auto metaObject = target->metaObject();

    m_signalData = getDataObject(target, notifySignal, options);

    bool foundProperties = false;
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        const auto property = metaObject->property(i);
        if (property.notifySignal() == notifySignal) {
            foundProperties = true;
            if (!m_signalData->contains(property)) {
                m_signalData->properties.push_back({property, property.read(target)});
            }
        }
    }

    if (!foundProperties) {
        qWarning() << "Error: Constructing NotifyGuard on signal that is not used for any properties.";
    }
}

Internal::SignalDataSPtr getDataObject(QObject* target, const QMetaMethod& notifySignal, NotifyGuard::GuardOptions options)
{
    auto createSignalData = [](QObject* target, const QMetaMethod& notifySignal) {
        Internal::SignalDataSPtr shared = std::make_shared<Internal::SignalData>();
        shared->target = target;
        shared->signal = notifySignal;
        return shared;
    };

    if (options == NotifyGuard::RecursiveScope) {
        const auto notifySignalIndex = notifySignal.methodIndex();

        auto instanceData = target->property(guardSharedDataPropertyName).value<Internal::InstanceDataPtr>();
        if (instanceData) {
            const auto it = std::find_if(instanceData->cbegin(), instanceData->cend(),
                                         [notifySignalIndex](const Internal::SignalIdDataPair& data){return notifySignalIndex == data.id;});
            if (it != instanceData->cend()) {
                if (auto sptr = it->dataWPtr.lock())
                   return sptr;
            }
        } else {
            instanceData = std::make_shared<Internal::InstanceData>();
            target->setProperty(guardSharedDataPropertyName, QVariant::fromValue(instanceData));
        }

        auto shared = createSignalData(target, notifySignal);
        instanceData->push_back({notifySignalIndex, Internal::SignalDataWPtr(shared)});
        return shared;
    } else {
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
    for (const auto& pp: properties) {
        const auto currentValue = pp.property.read(target);
        if (currentValue == pp.startValue)
            continue;
        // we need to emit the signal
        if (signal.parameterCount() == 0) {
            signal.invoke(target, Qt::DirectConnection);
        } else { // one parameter of the same type as the property itself
            signal.invoke(target, Qt::DirectConnection,
                          QGenericArgument(signal.parameterTypes().constFirst().data(), currentValue.data()));
        }
        break;
    }
}

bool Internal::SignalData::contains(const QMetaProperty& property) const
{
    const auto propertyIndex = property.propertyIndex();
    return std::any_of(properties.begin(), properties.end(), [propertyIndex](const PropertyPair& pp){
        return pp.property.propertyIndex() == propertyIndex;
    });
}

} //end namespace KDToolBox

Q_DECLARE_METATYPE(KDToolBox::Internal::InstanceDataPtr);

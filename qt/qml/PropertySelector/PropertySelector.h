/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2022 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#pragma once

#ifndef PROPERTYSWITCH_H
#define PROPERTYSWITCH_H

#include <bitset>
#include <private/qqmlcustomparser_p.h>
#include <QQmlParserStatus>


namespace cpp {
namespace qmltypes {

class PropertySelectorCustomParser;

class PropertySelector : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged)
    Q_INTERFACES(QQmlParserStatus)

public:
    ///@brief the maximum number of conditions supported
    static const int MaxConditionCount = 32;

    PropertySelector(QObject *parent = nullptr);

    QObject *target() const;
    Q_SLOT void setTarget(QObject *target);
    Q_SIGNAL void targetChanged(QObject *target);

    // QQmlParserStatus interface
    void componentComplete() override;
    void classBegin() override;

private: //types
    using ConditionBits = std::bitset<MaxConditionCount>;
    struct Rule {
        Rule() {}
        Rule(QStringList conditions, QString property, QVariant simpleValue, QV4::CompiledData::Location location)
            :conditions(std::move(conditions)), property(std::move(property)), simpleValue(simpleValue), location(location), binding(nullptr)
        {}
        Rule(QStringList conditions, QString property, const QV4::CompiledData::Binding *binding, QV4::CompiledData::Location location)
             :conditions(std::move(conditions)), property(std::move(property)), location(location), binding(binding), id(binding->value.compiledScriptIndex)
        {}

        QStringList conditions;
        QString property;
        QVariant simpleValue;

        QV4::CompiledData::Location location;
        const QV4::CompiledData::Binding * binding;
        QQmlBinding::Identifier id = QQmlBinding::Invalid;

        ConditionBits conditionBits;

        bool operator<(const Rule& other);
    };
    struct PropertyRules {
        QQmlProperty property;
        QVector<Rule>::const_iterator begin;
        QVector<Rule>::const_iterator end;
        QVector<Rule>::const_iterator currentRule;
        QQmlBinding *currentBinding;
    };

private: //methods
    void connectChangeSignals();
    Q_SLOT void apply();
    void applyRule(PropertyRules &propertyRules, QVector<Rule>::const_iterator rule);
    void prepareRules();
    ConditionBits bitmaskFromConditionList(const QStringList& conditions) const;

private: //members
    QObject *m_target = nullptr;
    QStringList m_conditions;
    QVector<Rule> m_ruleList;
    QQmlRefPointer<QV4::CompiledData::CompilationUnit> m_compilationUnit;
    QVector<PropertyRules> m_properties;

    friend class PropertySelectorCustomParser;
};


class PropertySelectorCustomParser: public QQmlCustomParser
{
public:
    // QQmlCustomParser interface
    void verifyBindings(const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compiledUnit, const QList<const QV4::CompiledData::Binding *> &bindings) override Q_DECL_FINAL;
    void applyBindings(QObject *, const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compiledUnit, const QList<const QV4::CompiledData::Binding *> &bindings) override Q_DECL_FINAL;

    void parseBinding(PropertySelector *selector, QStringList conditions, const QV4::CompiledData::Binding *binding);
};

}}
#endif // PROPERTYSWITCH_H

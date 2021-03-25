/****************************************************************************
**                                MIT License
**
** Copyright (C) 2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
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

#include "toContainer.h"

#include <QTest>

#include <QList>
#include <QVector>
#include <QSet>

#include <vector>
#include <deque>
#include <unordered_set>

#include <QDebug>

using namespace KDToolBox::Ranges;

class tst_toContainer : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void toContainer();

private:
    template <template <typename ...> class Container>
    void toContainer_impl();
};

void tst_toContainer::initTestCase()
{
    qDebug() << "Test built under C++" << __cplusplus;
}

void tst_toContainer::toContainer()
{
    toContainer_impl<QList>();
    toContainer_impl<QVector>();
    toContainer_impl<QSet>();
    toContainer_impl<std::vector>();
    toContainer_impl<std::deque>();
}

template <typename DestinationContainer,
          typename SourceContainer>
static void toContainer_helper(const SourceContainer &s)
{
    DestinationContainer d(s.begin(), s.end());
    QCOMPARE(kdToContainer<DestinationContainer>(s), d);
    QCOMPARE((s | kdToContainer<DestinationContainer>()), d);

    const auto functionReturningS = [&s]() { return s; };
    QCOMPARE(kdToContainer<DestinationContainer>(functionReturningS()), d);
    QCOMPARE((functionReturningS() | kdToContainer<DestinationContainer>()), d);
}

template <template <typename ...> class DestinationContainerTemplate,
          typename SourceContainer>
static void toContainer_helper2(const SourceContainer &s)
{
    DestinationContainerTemplate<typename SourceContainer::value_type> d(s.begin(), s.end());
    QCOMPARE(kdToContainer<DestinationContainerTemplate>(s), d);
    QCOMPARE((s | kdToContainer<DestinationContainerTemplate>()), d);

    const auto functionReturningS = [&s]() { return s; };
    QCOMPARE(kdToContainer<DestinationContainerTemplate>(functionReturningS()), d);
    QCOMPARE((functionReturningS() | kdToContainer<DestinationContainerTemplate>()), d);
}

template <template <typename ...> class Container>
void tst_toContainer::toContainer_impl()
{
    Container<int> container{1, 2, 3, 4, 5, 1, 2, 6, -1, 1, 2, 45};

    toContainer_helper<QList<int>>(container);
    toContainer_helper<QVector<int>>(container);
    toContainer_helper<QSet<int>>(container);
    toContainer_helper<std::vector<int>>(container);
    toContainer_helper<std::deque<int>>(container);
    toContainer_helper<std::unordered_set<int>>(container);

    // Same but without specifying's the container value type template argument
    toContainer_helper2<QList>(container);
    toContainer_helper2<QVector>(container);
    toContainer_helper2<QSet>(container);
    toContainer_helper2<std::vector>(container);
    toContainer_helper2<std::deque>(container);
    toContainer_helper2<std::unordered_set>(container);

    // Same but changing the value_type
    toContainer_helper<QList<double>>(container);
    toContainer_helper<QVector<double>>(container);
    toContainer_helper<QSet<double>>(container);
    toContainer_helper<std::vector<double>>(container);
    toContainer_helper<std::deque<double>>(container);
    toContainer_helper<std::unordered_set<double>>(container);
}

QTEST_MAIN(tst_toContainer)

#include "tst_toContainer.moc"

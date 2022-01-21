/****************************************************************************
**                                MIT License
**
** Copyright (C) 2021-2022 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Marc Mutz <marc.mutz@kdab.com>
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

#include "duplicatetracker.h"

#include <QTest>

#include <type_traits>

using namespace KDToolBox;

namespace {
template <typename DuplicateTracker> struct prealloc {};
template <typename T, size_t Prealloc, typename H, typename Eq>
struct prealloc<DuplicateTracker<T, Prealloc, H, Eq>>
    : std::integral_constant<std::size_t, Prealloc> {};
} // unnamed namespace

class tst_DuplicateTracker : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void defaultCtor();
    void reserve();
    void hasSeen();
};

void tst_DuplicateTracker::defaultCtor()
{
    DuplicateTracker<int> tracker;
    QVERIFY(tracker.set().empty());
    QVERIFY(tracker.set().bucket_count() >= prealloc<decltype(tracker)>::value);
}

void tst_DuplicateTracker::reserve()
{
    for (size_t i : {2, 13, 63, 64, 65, 1024}) {
        {
            DuplicateTracker<std::string> tracker(i);
            QVERIFY(tracker.set().bucket_count() >= i);
        }
        {
            DuplicateTracker<std::string> tracker;
            tracker.reserve(i);
            QVERIFY(tracker.set().bucket_count() >= i);
        }
    }
}

void tst_DuplicateTracker::hasSeen()
{
    DuplicateTracker<std::string> tracker;
    QVERIFY(!tracker.contains("hello"));
    QVERIFY(!tracker.hasSeen("hello"));
    QVERIFY( tracker.contains("hello"));
    QVERIFY( tracker.hasSeen("hello"));

    QVERIFY(!tracker.contains("world"));
    QVERIFY(!tracker.hasSeen("world"));
    QVERIFY( tracker.contains("world"));
    QVERIFY( tracker.hasSeen("world"));

    const auto exclamation = std::string("!");
    QVERIFY(!tracker.contains(exclamation));
    QVERIFY(!tracker.hasSeen(exclamation));
    QVERIFY( tracker.contains(exclamation));
    QVERIFY( tracker.hasSeen(exclamation));
}

QTEST_APPLESS_MAIN(tst_DuplicateTracker)

#include "tst_duplicatetracker.moc"

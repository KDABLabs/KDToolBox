/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marc Mutz <marc.mutz@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "duplicatetracker.h"

#include <QTest>

#include <type_traits>

using namespace KDToolBox;

namespace
{
template<typename DuplicateTracker>
struct prealloc
{
};
template<typename T, size_t Prealloc, typename H, typename Eq>
struct prealloc<DuplicateTracker<T, Prealloc, H, Eq>> : std::integral_constant<std::size_t, Prealloc>
{
};
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
    for (size_t i : {2, 13, 63, 64, 65, 1024})
    {
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
    QVERIFY(tracker.contains("hello"));
    QVERIFY(tracker.hasSeen("hello"));

    QVERIFY(!tracker.contains("world"));
    QVERIFY(!tracker.hasSeen("world"));
    QVERIFY(tracker.contains("world"));
    QVERIFY(tracker.hasSeen("world"));

    const auto exclamation = std::string("!");
    QVERIFY(!tracker.contains(exclamation));
    QVERIFY(!tracker.hasSeen(exclamation));
    QVERIFY(tracker.contains(exclamation));
    QVERIFY(tracker.hasSeen(exclamation));
}

QTEST_APPLESS_MAIN(tst_DuplicateTracker)

#include "tst_duplicatetracker.moc"

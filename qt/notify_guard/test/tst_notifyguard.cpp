/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/
#include <QSignalSpy>
#include <QTest>

#include "notifyguard.h"

class TestClass;
using namespace KDToolBox;

class NotifyGuardTest : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void test_noChange();
    void test_noArgument();
    void test_singleArgument();
    void test_PMFSyntax();
    void test_recursiveScope();
    void test_recursiveScope2();
    void test_recursiveScopePMF();
    void test_singleScope();
    void test_singleInsideRecursiveScope();
    void test_setResetInRecursiveScope();
    void test_invalidGuards();
    void test_moveFrom();
};

struct CustomType
{
    int iValue;
    friend bool operator==(CustomType lhs, CustomType rhs) noexcept { return lhs.iValue == rhs.iValue; }
    friend bool operator!=(CustomType lhs, CustomType rhs) noexcept { return lhs.iValue != rhs.iValue; }
};
Q_DECLARE_METATYPE(CustomType);

struct NonComparableType
{
    int iValue;
};
Q_DECLARE_METATYPE(NonComparableType);

class TestClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString stringProperty MEMBER m_stringProperty NOTIFY stringPropertyChanged)
    Q_PROPERTY(int intProperty READ intProperty NOTIFY intPropertyChanged)
    Q_PROPERTY(int intProperty1 MEMBER m_intProperty2 NOTIFY intPropertyChanged)
    Q_PROPERTY(CustomType customProperty MEMBER m_customProperty NOTIFY customPropertyChanged)
    Q_PROPERTY(NonComparableType nonComparableProperty READ nonComparableProperty WRITE setNonComparableProperty NOTIFY
                   nonComparablePropertyChanged)

public:
    using QObject::QObject;
    int intProperty() const { return m_intProperty; }

    NonComparableType nonComparableProperty() const { return m_nonComparable; }

    void setNonComparableProperty(const NonComparableType &value)
    {
        if (value.iValue != m_nonComparable.iValue)
        {
            m_nonComparable = value;
            Q_EMIT nonComparablePropertyChanged();
        }
    }

Q_SIGNALS:
    void stringPropertyChanged(QString);
    void intPropertyChanged();
    void customPropertyChanged();
    void nonComparablePropertyChanged();
    void nonPropertySignal();

public:
    QString m_stringProperty;
    int m_intProperty = 0;
    int m_intProperty2 = 0;
    CustomType m_customProperty{0};
    NonComparableType m_nonComparable{0};
};

void NotifyGuardTest::test_noChange()
{
    TestClass test;
    QSignalSpy stringSpy(&test, &TestClass::stringPropertyChanged);
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard stringGuard(&test, "stringProperty");
        NotifyGuard intGuard(&test, "intProperty");
        QVERIFY(stringGuard.isActive());
        QVERIFY(intGuard.isActive());
    }

    QVERIFY(stringSpy.isEmpty());
    QVERIFY(intSpy.isEmpty());
}

void NotifyGuardTest::test_noArgument()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard intGuard(&test, "intProperty");
        QVERIFY(intGuard.isActive());
        test.m_intProperty = 42;
    }

    QCOMPARE(intSpy.count(), 1);
}

void NotifyGuardTest::test_singleArgument()
{
    TestClass test;
    QSignalSpy stringSpy(&test, &TestClass::stringPropertyChanged);
    const QString testString = QStringLiteral("Some test string");
    {
        NotifyGuard stringGuard(&test, "stringProperty");
        QVERIFY(stringGuard.isActive());
        test.m_stringProperty = testString;
    }

    QCOMPARE(stringSpy.count(), 1);
    QCOMPARE(stringSpy.at(0).at(0).toString(), testString);
}

void NotifyGuardTest::test_PMFSyntax()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);

    // change int 1
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        QVERIFY(guard.isActive());
        test.m_intProperty++;
    }
    QCOMPARE(intSpy.count(), 1);
    intSpy.clear();

    // change int 2
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        QVERIFY(guard.isActive());
        test.m_intProperty2++;
    }
    QCOMPARE(intSpy.count(), 1);
    intSpy.clear();

    // change both ints
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        QVERIFY(guard.isActive());
        test.m_intProperty++;
        test.m_intProperty2++;
    }
    QCOMPARE(intSpy.count(), 1);
    intSpy.clear();

    // no change
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        QVERIFY(guard.isActive());
    }
    QCOMPARE(intSpy.count(), 0);
    intSpy.clear();
}

void NotifyGuardTest::test_recursiveScope()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard outerGuard(&test, "intProperty", NotifyGuard::RecursiveScope);
        QVERIFY(outerGuard.isActive());
        {
            NotifyGuard innerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty2 = 11;
            // innerGuard should _not_ emit, as intProperty2 uses the same notification signal as intProperty which is
            // already in a scope
        }
        QVERIFY(intSpy.isEmpty());
        test.m_intProperty = 42;
    }

    QCOMPARE(intSpy.count(), 1);
}

void NotifyGuardTest::test_recursiveScope2()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard outerGuard(&test, "intProperty", NotifyGuard::RecursiveScope);
        QVERIFY(outerGuard.isActive());
        {
            NotifyGuard innerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty2 = 11;
            // innerGuard should _not_ emit, as intProperty2 uses the same notification signal as intProperty which is
            // already in a scope
        }
        QVERIFY(intSpy.isEmpty());
    }

    // only on the outer guard should the signal emit, even though the guard on the changed property is the innerGuard
    QCOMPARE(intSpy.count(), 1);
}

void NotifyGuardTest::test_recursiveScopePMF()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard outerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
        QVERIFY(outerGuard.isActive());
        {
            NotifyGuard innerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty2 = 11;
            test.m_intProperty = 12;
            // innerGuard should _not_ emit, as intProperty2 uses the same notification signal as intProperty which is
            // already in a scope
        }
        QVERIFY(intSpy.isEmpty());
        test.m_intProperty = 42;
    }

    // only on the outer guard should the signal emit, even though the guard on the changed property is the innerGuard
    QCOMPARE(intSpy.count(), 1);

    // test second iteration. Uncovered a bug that caused the implementation to work the first time, but not a second
    // time, so testing for that.
    test.m_intProperty = 0;
    test.m_intProperty2 = 0;
    intSpy.clear();

    {
        NotifyGuard outerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
        QVERIFY(outerGuard.isActive());
        {
            NotifyGuard innerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty2 = 11;
            test.m_intProperty = 12;
            // innerGuard should _not_ emit, as intProperty2 uses the same notification signal as intProperty which is
            // already in a scope
        }
        QVERIFY(intSpy.isEmpty());
        test.m_intProperty = 42;
    }

    // only on the outer guard should the signal emit, even though the guard on the changed property is the innerGuard
    QCOMPARE(intSpy.count(), 1);
}

void NotifyGuardTest::test_singleScope()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard outerGuard(&test, "intProperty", NotifyGuard::SingleScope);
        QVERIFY(outerGuard.isActive());
        {
            NotifyGuard innerGuard(&test, "intProperty", NotifyGuard::SingleScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty = 11;
            // innerGuards _should_ emit
        }
        QCOMPARE(intSpy.count(), 1);
        test.m_intProperty = 42;
    }

    QCOMPARE(intSpy.count(), 2);
}

void NotifyGuardTest::test_singleInsideRecursiveScope()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard outerGuard(&test, "intProperty", NotifyGuard::RecursiveScope);
        QVERIFY(outerGuard.isActive());
        {
            NotifyGuard innerGuard(&test, "intProperty", NotifyGuard::SingleScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty = 11;
            // innerGuards _should_ emit
        }
        QCOMPARE(intSpy.count(), 1);
        test.m_intProperty = 42;
    }

    QCOMPARE(intSpy.count(), 2);
}

void NotifyGuardTest::test_setResetInRecursiveScope()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard outerGuard(&test, "intProperty", NotifyGuard::RecursiveScope);
        {
            NotifyGuard innerGuard(&test, "intProperty1", NotifyGuard::RecursiveScope);
            QVERIFY(innerGuard.isActive());
            test.m_intProperty = 11;
        }
        QVERIFY(outerGuard.isActive());

        QVERIFY(intSpy.isEmpty());

        {
            NotifyGuard innerGuard(&test, "intProperty1", NotifyGuard::RecursiveScope);
            test.m_intProperty = 0;
        }
        QVERIFY(intSpy.isEmpty());

        test.m_intProperty = 42;
    }

    // This is a bit of a weird one actually. Would we expect signals to be emitted here or not? How many?
    QCOMPARE(intSpy.count(), 1);
}

void NotifyGuardTest::test_invalidGuards()
{
    TestClass test;

    {
        NotifyGuard guard;
        QVERIFY(!guard.isActive());
    }

    qDebug() << "Note: we expect some warning messages below; we're testing expected fail cases.";
    {
        NotifyGuard guard(&test, "invalidProperty");
        QVERIFY(!guard.isActive());
    }

    {
        NotifyGuard guard(&test, &TestClass::nonPropertySignal);
        QVERIFY(!guard.isActive());
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // try with unregistered custom type. There is no need for explicit registration with Qt 6, so guard construction
    // would fail-to-fail.
    {
        NotifyGuard guard(&test, "customProperty");
        QVERIFY(!guard.isActive());
    }

    {
        NotifyGuard guard(&test, &TestClass::customPropertyChanged);
        QVERIFY(!guard.isActive());
    }

    // now, register the comparator, and try again
    QMetaType::registerEqualsComparator<CustomType>();
#else
    // try with non-comparable type. This one should not be auto-registered with Qt 6
    {
        NotifyGuard guard(&test, "nonComparableProperty");
        QVERIFY(!guard.isActive());
    }

    {
        NotifyGuard guard(&test, &TestClass::nonComparablePropertyChanged);
        QVERIFY(!guard.isActive());
    }
#endif
    qDebug() << "End of expected warning messages.";

    {
        NotifyGuard guard(&test, "customProperty");
        QVERIFY(guard.isActive());
    }

    {
        NotifyGuard guard(&test, &TestClass::customPropertyChanged);
        QVERIFY(guard.isActive());
    }
}

void NotifyGuardTest::test_moveFrom()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    QSignalSpy stringSpy(&test, &TestClass::stringPropertyChanged);
    {
        NotifyGuard guard;
        QVERIFY(!guard.isActive());
        if (intSpy.isEmpty())
        {
            guard = NotifyGuard(&test, "intProperty");
        }
        else
        {
            guard = NotifyGuard(&test, "stringProperty");
        }
        QVERIFY(guard.isActive());

        test.m_intProperty = 42;
    }

    QCOMPARE(intSpy.count(), 1);
    QCOMPARE(stringSpy.count(), 0);
}

QTEST_MAIN(NotifyGuardTest)

#include "tst_notifyguard.moc"

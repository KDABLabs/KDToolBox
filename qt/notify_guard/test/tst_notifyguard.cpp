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
#include <QtTest>

#include "notifyguard.h"

class TestClass;
using namespace KDToolBox;

class NotifyGuardTest : public QObject
{
    Q_OBJECT

public:
    NotifyGuardTest();

private Q_SLOTS:
    void test_noChange();
    void test_noArgument();
    void test_singleArgument();
    void test_PMFSyntax();
    void test_recursiveScope();
    void test_recursiveScope2();
    void test_singleScope();
    void test_singleInsideRecursiveScope();
    void test_setResetInRecursiveScope();
    void test_moveFrom();
};

class TestClass: public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString stringProperty MEMBER m_stringProperty NOTIFY stringPropertyChanged)
    Q_PROPERTY (int intProperty READ intProperty NOTIFY intPropertyChanged)
    Q_PROPERTY (int intProperty1 MEMBER m_intProperty2 NOTIFY intPropertyChanged)

public:
    int intProperty() const {return m_intProperty;}

Q_SIGNALS:
    void stringPropertyChanged(QString);
    void intPropertyChanged();

public:
    QString m_stringProperty;
    int m_intProperty = 0;
    int m_intProperty2 = 0;
};

NotifyGuardTest::NotifyGuardTest()
{
}

void NotifyGuardTest::test_noChange()
{
    TestClass test;
    QSignalSpy stringSpy(&test, &TestClass::stringPropertyChanged);
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    {
        NotifyGuard stringGuard(&test, "stringProperty");
        NotifyGuard intGuard(&test, "intProperty");
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
        test.m_stringProperty = testString;
    }

    QCOMPARE(stringSpy.count(), 1);
    QCOMPARE(stringSpy.at(0).at(0).toString(), testString);
}

void NotifyGuardTest::test_PMFSyntax()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);

    //change int 1
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        test.m_intProperty++;
    }
    QCOMPARE(intSpy.count(), 1);
    intSpy.clear();

    //change int 2
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        test.m_intProperty2++;
    }
    QCOMPARE(intSpy.count(), 1);
    intSpy.clear();

    //change both ints
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
        test.m_intProperty++;
        test.m_intProperty2++;
    }
    QCOMPARE(intSpy.count(), 1);
    intSpy.clear();

    //no change
    {
        NotifyGuard guard(&test, &TestClass::intPropertyChanged);
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
        {
            NotifyGuard innerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
            test.m_intProperty2 = 11;
            // innerGuard should _not_ emit, as intProperty2 uses the same notification signal as intProperty which is already in a scope
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
        {
            NotifyGuard innerGuard(&test, &TestClass::intPropertyChanged, NotifyGuard::RecursiveScope);
            test.m_intProperty2 = 11;
            // innerGuard should _not_ emit, as intProperty2 uses the same notification signal as intProperty which is already in a scope
        }
        QVERIFY(intSpy.isEmpty());
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
        {
            NotifyGuard innerGuard(&test, "intProperty", NotifyGuard::SingleScope);
            test.m_intProperty = 11;
            //innerGuards _should_ emit
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
        {
            NotifyGuard innerGuard(&test, "intProperty", NotifyGuard::SingleScope);
            test.m_intProperty = 11;
            //innerGuards _should_ emit
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
            test.m_intProperty = 11;
        }
        QVERIFY(intSpy.isEmpty());

        {
            NotifyGuard innerGuard(&test, "intProperty1", NotifyGuard::RecursiveScope);
            test.m_intProperty = 0;
        }
        QVERIFY(intSpy.isEmpty());

        test.m_intProperty = 42;
    }

    //This is a bit of a weird one actually. Would we expect signals to be emitted here or not? How many?
    QCOMPARE(intSpy.count(), 1);
}

void NotifyGuardTest::test_moveFrom()
{
    TestClass test;
    QSignalSpy intSpy(&test, &TestClass::intPropertyChanged);
    QSignalSpy stringSpy(&test, &TestClass::stringPropertyChanged);
    {
        NotifyGuard guard;
        if (intSpy.isEmpty()) {
            guard = NotifyGuard(&test, "intProperty");
        } else {
            guard = NotifyGuard(&test, "stringProperty");
        }

        test.m_intProperty = 42;
    }

    QCOMPARE(intSpy.count(), 1);
    QCOMPARE(stringSpy.count(), 0);
}


QTEST_MAIN(NotifyGuardTest)

#include "tst_notifyguard.moc"

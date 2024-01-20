/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../propagate_const.h"
#include <memory>

#include <QTest>

// clang-format off

using namespace KDToolBox;

class tst_propagate_const : public QObject
{
    Q_OBJECT

public:
    explicit tst_propagate_const(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:
    void basicTests();
    void inheritanceTests();
    void conversionTests();

private:
    template <template <typename> typename PtrAdaptor>
    void basicTests_impl();

    template <template <typename> typename PtrAdaptor>
    void inheritanceTests_impl();

    class Incomplete;
    propagate_const<Incomplete *> m_incompleteTest; // compile-only test
};

class Base
{
public:
    Base() = default;
    virtual ~Base() {}
    Base(const Base &) = delete;
    Base(Base &&) = delete;
    Base &operator=(const Base &) = delete;
    Base &operator=(Base &&) = delete;

    void shouldBeCalledConst() { QFAIL("Error"); }
    void shouldBeCalledConst() const {}

    void shouldBeCalledNonConst() {}
    void shouldBeCalledNonConst() const { QFAIL("Error"); }

    int a = 42;
};

class Derived : public Base
{
public:
    Derived() = default;

    int b = 123;
};

void baseFunction(Base *) {}
void baseConstFunction(const Base *) {}


template <typename T>
struct RawPointerAdaptor
{
    using type = T *;

    // This leaks memory, but it's just for a test.

    type get() { delete m_ptr; m_ptr = new T(); return m_ptr; }

    T *rawGet() { return m_ptr; }

    T *m_ptr = nullptr;
};

template <typename T>
struct UniquePointerAdaptor
{
    using type = std::unique_ptr<T>;

    type get()
    {
        auto result = std::make_unique<T>();
        m_ptr = result.get();
        return result;
    }

    T *rawGet() { return m_ptr; }
    T *m_ptr = nullptr;
};


template <template <typename> typename PtrAdaptor>
void tst_propagate_const::basicTests_impl()
{
    PtrAdaptor<int> adaptor;
    using Ptr = typename PtrAdaptor<int>::type;

    {
        [[maybe_unused]] propagate_const<Ptr> pc;
        propagate_const<Ptr> pc2{};
        propagate_const<Ptr> pc3{nullptr};
        propagate_const<Ptr> pc4 = nullptr;

        QVERIFY(!pc2);
        QVERIFY(!pc3);
        QVERIFY(!pc4);

        QVERIFY(pc2 == nullptr);
        QVERIFY(pc3 == nullptr);
        QVERIFY(pc4 == nullptr);

        QCOMPARE(pc2.get(), nullptr);
        QCOMPARE(pc3.get(), nullptr);
        QCOMPARE(pc4.get(), nullptr);
    }
    {
        propagate_const<Ptr> pc = adaptor.get();
        QVERIFY(pc);
        QCOMPARE(pc.get(), adaptor.rawGet());
        *pc = 42;
        QCOMPARE(*pc, 42);
        QCOMPARE(*pc.get(), 42);
        static_assert(std::is_same_v<decltype(pc.get()), int *>);
        static_assert(std::is_same_v<decltype(get_underlying(pc)), Ptr &>);
    }
    {
        const propagate_const<Ptr> pc = adaptor.get();
        QVERIFY(pc);
        QCOMPARE(pc.get(), adaptor.rawGet());
        QCOMPARE(*pc, 0);
        QCOMPARE(*pc.get(), 0);
        static_assert(std::is_same_v<decltype(pc.get()), const int *>);
        static_assert(std::is_same_v<decltype(get_underlying(pc)), const Ptr &>);
    }
    {
        PtrAdaptor<int> adaptor1;
        propagate_const<Ptr> pc1 = adaptor1.get();
        *pc1 = 42;

        PtrAdaptor<int> adaptor2;
        propagate_const<Ptr> pc2 = adaptor2.get();
        *pc2 = 123;

        using std::swap;
        swap(pc1, pc2);

        QCOMPARE(*pc1, 123);
        QCOMPARE(*pc2, 42);
    }
}

template <template <typename> typename PtrAdaptor>
void tst_propagate_const::inheritanceTests_impl()
{
    PtrAdaptor<Base> baseAdaptor;
    PtrAdaptor<Derived> derivedAdaptor;

    using BasePtr = typename PtrAdaptor<Base>::type;
    using DerivedPtr = typename PtrAdaptor<Derived>::type;

    {
        [[maybe_unused]] propagate_const<BasePtr> pc1; // uninitialized!
        propagate_const<BasePtr> pc2{};
        propagate_const<BasePtr> pc3 = nullptr;
        propagate_const<BasePtr> pc4{nullptr};

        QCOMPARE(pc2.get(), nullptr);
        QCOMPARE(pc3.get(), nullptr);
        QCOMPARE(pc4.get(), nullptr);
    }

    {
        propagate_const<BasePtr> pc{baseAdaptor.get()};
        QVERIFY(pc.get() == baseAdaptor.rawGet());
        QVERIFY(pc.get() != nullptr);
    }
    {
        const propagate_const<BasePtr> pc{baseAdaptor.get()};
        QVERIFY(pc.get() == baseAdaptor.rawGet());
        QVERIFY(pc.get() != nullptr);
    }
    {
        propagate_const<BasePtr> pc{derivedAdaptor.get()};
        QVERIFY(pc.get() == derivedAdaptor.rawGet());
        QVERIFY(pc.get() != nullptr);
    }
    {
        const propagate_const<BasePtr> pc{derivedAdaptor.get()};
        QVERIFY(pc.get() == derivedAdaptor.rawGet());
        QVERIFY(pc.get() != nullptr);
    }
    {
        propagate_const<DerivedPtr> pc{derivedAdaptor.get()};
        QVERIFY(pc.get() == derivedAdaptor.rawGet());
        QVERIFY(pc.get() != nullptr);
    }
    {
        const propagate_const<DerivedPtr> pc{derivedAdaptor.get()};
        QVERIFY(pc.get() == derivedAdaptor.rawGet());
        QVERIFY(pc.get() != nullptr);
    }
    {
        propagate_const<DerivedPtr> pc1{derivedAdaptor.get()};
        QVERIFY(pc1.get() == derivedAdaptor.rawGet());
        QVERIFY(pc1.get() != nullptr);

        propagate_const<DerivedPtr> pc2{std::move(pc1)};
        QVERIFY(pc2.get() == derivedAdaptor.rawGet());
        QVERIFY(pc2.get() != nullptr);

        propagate_const<BasePtr> pc3{std::move(pc2)};
        QVERIFY(pc3.get() == derivedAdaptor.rawGet());
        QVERIFY(pc3.get() != nullptr);
    }
    {
        propagate_const<BasePtr> pc{baseAdaptor.get()};
        pc->shouldBeCalledNonConst();
    }
    {
        propagate_const<BasePtr> pc{derivedAdaptor.get()};
        pc->shouldBeCalledNonConst();
    }
    {
        const propagate_const<BasePtr> pc{baseAdaptor.get()};
        pc->shouldBeCalledConst();
    }
    {
        const propagate_const<BasePtr> pc{derivedAdaptor.get()};
        pc->shouldBeCalledConst();
    }
}

void tst_propagate_const::basicTests()
{
    basicTests_impl<RawPointerAdaptor>();
    basicTests_impl<UniquePointerAdaptor>();
}

void tst_propagate_const::inheritanceTests()
{
    inheritanceTests_impl<RawPointerAdaptor>();
    inheritanceTests_impl<UniquePointerAdaptor>();
}

void tst_propagate_const::conversionTests()
{
    static_assert(std::is_convertible_v<propagate_const<int *>, int *>);
    static_assert(std::is_convertible_v<propagate_const<int *>, const int *>);
    static_assert(!std::is_convertible_v<const propagate_const<int *>, int *>);
    static_assert(std::is_convertible_v<const propagate_const<int *>, const int *>);

    {
        propagate_const<Derived *> pc{};
        baseFunction(pc);
        baseConstFunction(pc);
    }
    {
        const propagate_const<Derived *> pc{};
        baseConstFunction(pc);
    }
    {
        int array[3] = {};
        propagate_const<int *> pa(array);
        *(pa + 1) = 42;
        QCOMPARE(array[0], 0);
        QCOMPARE(array[1], 42);
        QCOMPARE(array[2], 0);
    }
    {
        propagate_const<int *> pc(new int);
        delete +pc;
    }
}

QTEST_MAIN(tst_propagate_const)

#include "tst_propagate_const.moc"

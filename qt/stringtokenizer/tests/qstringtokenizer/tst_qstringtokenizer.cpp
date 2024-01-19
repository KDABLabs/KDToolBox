/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QStringBuilder>
#include <QStringTokenizer>

#include <QTest>

#include <string>

Q_DECLARE_METATYPE(Qt::SplitBehavior)

class tst_QStringTokenizer : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void constExpr() const;
    void basics_data() const;
    void basics() const;
    void toContainer() const;
};

static QStringList skipped(const QStringList &sl)
{
    QStringList result;
    result.reserve(sl.size());
    for (const QString &s : sl)
    {
        if (!s.isEmpty())
            result.push_back(s);
    }
    return result;
}

QString toQString(QStringView str)
{
    return str.toString();
}

template<typename Container>
QStringList toQStringList(const Container &c)
{
    QStringList r;
    for (auto &&e : c)
        r.push_back(toQString(e));
    return r;
}

void tst_QStringTokenizer::constExpr() const
{
#ifdef Q_COMPILER_CONSTEXPR
    // compile-time checks
    {
        constexpr auto tok = qTokenize(u"a,b,c", u",");
        Q_UNUSED(tok);
    }
    {
        constexpr auto tok = qTokenize(u"a,b,c", u',');
        Q_UNUSED(tok);
    }
#else
    QSKIP("This test requires C++11 constexpr support enabled in the compiler.");
#endif
}

void tst_QStringTokenizer::basics_data() const
{
    QTest::addColumn<Qt::SplitBehavior>("sb");
    QTest::addColumn<Qt::CaseSensitivity>("cs");

#define ROW(sb, cs)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        QTest::addRow("%s/%s", #sb, #cs) << Qt::SplitBehavior{Qt::sb} << Qt::cs;                                       \
    } while (0)

    ROW(KeepEmptyParts, CaseSensitive);
    ROW(KeepEmptyParts, CaseInsensitive);
    ROW(SkipEmptyParts, CaseSensitive);
    ROW(SkipEmptyParts, CaseInsensitive);

#undef ROW
}

void tst_QStringTokenizer::basics() const
{
    QFETCH(const Qt::SplitBehavior, sb);
    QFETCH(const Qt::CaseSensitivity, cs);

    auto expected = QStringList{
        QLatin1String(""),  QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"),
        QStringLiteral("d"), QStringLiteral("e"), QLatin1String(""),
    };
    if (sb & Qt::SkipEmptyParts)
        expected = skipped(expected);
    QCOMPARE(toQStringList(qTokenize(u",a,b,c,d,e,", u',', sb, cs)), expected);
    QCOMPARE(toQStringList(qTokenize(u",a,b,c,d,e,", u',', cs, sb)), expected);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    {
        auto tok = qTokenize(expected.join(u'x'), u"X" % QString(), Qt::CaseInsensitive);
        // the temporary QStrings returned from join() and the QStringBuilder expression
        // are now destroyed, but 'tok' should keep both alive
        QCOMPARE(toQStringList(tok), expected);
    }
#endif

#ifdef __cpp_lib_chrono_udls
    using namespace std::string_literals;

    {
        auto tok = qTokenize(expected.join(u'x'), u"X"s, Qt::CaseInsensitive);
        QCOMPARE(toQStringList(tok), expected);
    }
#endif

    {
        auto tok = qTokenize(expected.join(u'x'), QLatin1Char('x'), cs, sb);
        QCOMPARE(toQStringList(tok), expected);
    }
}

void tst_QStringTokenizer::toContainer() const
{
    // QStringView value_type:
    {
        auto tok = qTokenize(u"a,b,c", u',');
        auto v = tok.toContainer();
        Q_UNUSED(v);
        QVERIFY((std::is_same<decltype(v), QVector<QStringView>>::value));
    }
    // QLatin1String value_type
    {
        auto tok = qTokenize(QLatin1String{"a,b,c"}, u',');
        auto v = tok.toContainer();
        Q_UNUSED(v);
        QVERIFY((std::is_same<decltype(v), QVector<QLatin1String>>::value));
    }
}

QTEST_APPLESS_MAIN(tst_QStringTokenizer)
#include "tst_qstringtokenizer.moc"

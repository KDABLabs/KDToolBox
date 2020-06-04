/****************************************************************************
**                                MIT License
**
** Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Marc Mutz <marc.mutz@kdab.com>
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

#include <QStringTokenizer>
#include <QStringBuilder>

#include <QTest>

#include <string>

Q_DECLARE_METATYPE(Qt::SplitBehavior)

class tst_QStringTokenizer : public QObject
{
    Q_OBJECT

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
    for (const QString &s : sl) {
        if (!s.isEmpty())
            result.push_back(s);
    }
    return result;
}

QString toQString(QStringView str)
{
    return str.toString();
}

template <typename Container>
QStringList toQStringList(const Container &c)
{
    QStringList r;
    for (auto &&e : c)
        r.push_back(toQString(e));
    return r;
}

void tst_QStringTokenizer::constExpr() const
{
    // compile-time checks
    {
        constexpr auto tok = qTokenize(u"a,b,c", u",");
        Q_UNUSED(tok);
    }
    {
        constexpr auto tok = qTokenize(u"a,b,c", u',');
        Q_UNUSED(tok);
    }
}

void tst_QStringTokenizer::basics_data() const
{
    QTest::addColumn<Qt::SplitBehavior>("sb");
    QTest::addColumn<Qt::CaseSensitivity>("cs");

#define ROW(sb, cs) \
    do { QTest::addRow("%s/%s", #sb, #cs) << Qt::SplitBehavior{Qt::sb} << Qt::cs; } while (0)

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

    auto expected = QStringList{"", "a", "b", "c", "d", "e", ""};
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
        QVERIFY((std::is_same<decltype(v), QVector<QStringView>>::value));
    }
    // QLatin1String value_type
    {
        auto tok = qTokenize(QLatin1String{"a,b,c"}, u',');
        auto v = tok.toContainer();
        QVERIFY((std::is_same<decltype(v), QVector<QLatin1String>>::value));
    }
}

QTEST_APPLESS_MAIN(tst_QStringTokenizer)
#include "tst_qstringtokenizer.moc"

/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <KDFunctionalSortFilterProxyModel.h>

#include <QStandardItemModel>
#include <QTest>

#include <QColor>
#include <QStringListModel>

#include <memory>

class tst_KDFunctionalSortFilterProxyModel : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void filterAcceptsRows();
    void filterAcceptsColumns();
    void sort();
    void filterAndSort();

private:
    static std::unique_ptr<QStandardItemModel> createModel(int rows, int columns);
};

void tst_KDFunctionalSortFilterProxyModel::filterAcceptsRows()
{
    constexpr int Rows = 100;
    constexpr int Columns = 3;

    auto model = createModel(Rows, Columns);

    KDFunctionalSortFilterProxyModel proxy;
    proxy.setSourceModel(model.get());

    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);

    auto beginsWithOne = [](const QAbstractItemModel *model, int source_row, const QModelIndex &parent) {
        return model->index(source_row, 0, parent).data().toString().startsWith(u'1');
    };

    proxy.setFilterAcceptsRowFunction(beginsWithOne);
    QCOMPARE(proxy.rowCount(), 11);
    QCOMPARE(proxy.columnCount(), Columns);

    QCOMPARE(proxy.index(0, 0).data().toString(), QStringLiteral("1-0"));
    QCOMPARE(proxy.index(0, 1).data().toString(), QStringLiteral("1-1"));
    QCOMPARE(proxy.index(0, 2).data().toString(), QStringLiteral("1-2"));

    for (int row = 1; row <= 10; ++row)
    {
        QCOMPARE(proxy.index(row, 0).data().toString(), QStringLiteral("%1-0").arg(row + 9));
        QCOMPARE(proxy.index(row, 1).data().toString(), QStringLiteral("%1-1").arg(row + 9));
        QCOMPARE(proxy.index(row, 2).data().toString(), QStringLiteral("%1-2").arg(row + 9));
    }

    proxy.clearFilterAcceptsRowFunction();
    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
}

void tst_KDFunctionalSortFilterProxyModel::filterAcceptsColumns()
{
    constexpr int Rows = 100;
    constexpr int Columns = 3;

    auto model = createModel(Rows, Columns);

    KDFunctionalSortFilterProxyModel proxy;
    proxy.setSourceModel(model.get());

    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);

    auto keepLastColumn = [](const QAbstractItemModel *model, int source_column, const QModelIndex &parent) {
        return source_column == (model->columnCount(parent) - 1);
    };

    proxy.setFilterAcceptsColumnFunction(keepLastColumn);

    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), 1);

    for (int row = 0; row < Rows; ++row)
    {
        QCOMPARE(proxy.index(row, 0).data().toString(), QStringLiteral("%1-%2").arg(row).arg(Columns - 1));
    }

    proxy.clearFilterAcceptsColumnFunction();
    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
}

void tst_KDFunctionalSortFilterProxyModel::sort()
{
    constexpr int Rows = 100;
    constexpr int Columns = 3;

    auto model = createModel(Rows, Columns);

    KDFunctionalSortFilterProxyModel proxy;
    proxy.setSourceModel(model.get());

    const auto verifyDefaultContents = [&]() {
        for (int row = 0; row < Rows; ++row)
        {
            for (int column = 0; column < Columns; ++column)
            {
                QCOMPARE(proxy.index(row, column).data().toString(), QStringLiteral("%1-%2").arg(row).arg(column));
            }
        }
    };

    const auto evenBeforeOdds = [](const QModelIndex &lhs, const QModelIndex &rhs) {
        auto getNumberBeforeDash = [](const QModelIndex &index) {
            const auto contents = index.data().toString();
            const auto dash = contents.indexOf(u'-');
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            return contents.leftRef(dash).toInt();
#else
            return QStringView(contents).left(dash).toInt();
#endif
        };

        auto lhsIsEven = getNumberBeforeDash(lhs) % 2 == 0;
        auto rhsIsEven = getNumberBeforeDash(rhs) % 2 == 0;

        return lhsIsEven && !rhsIsEven;
    };

    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
    verifyDefaultContents();

    proxy.setLessThanFunction(evenBeforeOdds);
    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
    verifyDefaultContents();

    proxy.sort(0);
    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
    for (int row = 0; row < Rows; ++row)
    {
        for (int column = 0; column < Columns; ++column)
        {
            // 0, 2, 4, ..., 98, 1, 3, 5, 7, ..., 99
            const int expectedRow = (row < 50) ? row * 2 : (row - 50) * 2 + 1;
            QCOMPARE(proxy.index(row, column).data().toString(), QStringLiteral("%1-%2").arg(expectedRow).arg(column));
        }
    }

    proxy.sort(-1);
    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
    verifyDefaultContents();

    proxy.clearLessThanFunction();
    QCOMPARE(proxy.rowCount(), Rows);
    QCOMPARE(proxy.columnCount(), Columns);
    verifyDefaultContents();
}

void tst_KDFunctionalSortFilterProxyModel::filterAndSort()
{
    QStringListModel model;
    model.setStringList(QColor::colorNames());

    KDFunctionalSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    QCOMPARE(proxy.rowCount(), model.stringList().size());
    QCOMPARE(proxy.columnCount(), 1);

    const auto isVowel = [](int letter) {
        switch (letter)
        {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
        case 'y':
            return true;
        default:
            return false;
        }
    };

    const auto onlyVowelColors = [&](const QAbstractItemModel *model, int source_row, const QModelIndex &parent) {
        const auto data = model->index(source_row, 0, parent).data().toString();
        const auto firstLetter = data.front().unicode();
        return isVowel(firstLetter);
    };

    proxy.setFilterAcceptsRowFunction(onlyVowelColors);

    QCOMPARE(proxy.columnCount(), 1);
    int rowCount = proxy.rowCount();
    QVERIFY(rowCount > 0);

    for (int row = 0; row < rowCount; ++row)
    {
        const auto data = proxy.index(row, 0).data().toString();
        QVERIFY(isVowel(data.front().unicode()));
    }

    const auto sortByLength = [&](const QModelIndex &lhs, const QModelIndex &rhs) {
        return lhs.data().toString().size() < rhs.data().toString().size();
    };

    proxy.setLessThanFunction(sortByLength);
    proxy.sort(0);

    QCOMPARE(proxy.columnCount(), 1);
    rowCount = proxy.rowCount();
    QVERIFY(rowCount > 0);
    int sizeSoFar = proxy.index(0, 0).data().toString().size();

    for (int row = 0; row < rowCount; ++row)
    {
        const auto data = proxy.index(row, 0).data().toString();
        QVERIFY(isVowel(data.front().unicode()));
        QVERIFY(data.size() >= sizeSoFar);
        sizeSoFar = data.size();
    }

    // clear filtering, make sure it's still sorted
    proxy.clearFilterAcceptsRowFunction();

    QCOMPARE(proxy.columnCount(), 1);
    rowCount = proxy.rowCount();
    QVERIFY(rowCount > 0);
    sizeSoFar = proxy.index(0, 0).data().toString().size();
    for (int row = 0; row < rowCount; ++row)
    {
        const auto data = proxy.index(row, 0).data().toString();
        QVERIFY(data.size() >= sizeSoFar);
        sizeSoFar = data.size();
    }
}

std::unique_ptr<QStandardItemModel> tst_KDFunctionalSortFilterProxyModel::createModel(int rows, int columns)
{
    std::unique_ptr<QStandardItemModel> model(new QStandardItemModel(rows, columns));

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            model->setItem(row, column, new QStandardItem(QStringLiteral("%1-%2").arg(row).arg(column)));
        }
    }

    return model;
}

QTEST_GUILESS_MAIN(tst_KDFunctionalSortFilterProxyModel)

#include "tst_kdfunctionalsortfilterproxymodel.moc"

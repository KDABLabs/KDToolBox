/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <KDTableToListProxyModel.h>

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include <algorithm>
#include <memory>
#include <vector>

class tst_KDTableToListProxyModel : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private Q_SLOTS:
    void initTestCase();

    void testEmptyProxy();
    void testNonEmptyProxy();
    void simpleMapping();

    void dataChanged();
    void rowManipulation();
    void columnManipulation();
    void layoutManipulation();

    void setData();
    void setItemData();

private:
    static std::unique_ptr<QStandardItemModel> createModel(int rows, int columns);
};

void tst_KDTableToListProxyModel::initTestCase()
{
    qRegisterMetaType<QList<QPersistentModelIndex>>();
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>();
}

void tst_KDTableToListProxyModel::testEmptyProxy()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);

    QCOMPARE(proxy.rowCount(), 0);
    QCOMPARE(proxy.columnCount(), 0);

    QStandardItemModel model;
    proxy.setSourceModel(&model);

    QCOMPARE(proxy.rowCount(), 0);
    QCOMPARE(proxy.columnCount(), 1);
}

void tst_KDTableToListProxyModel::testNonEmptyProxy()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);

    QCOMPARE(proxy.rowCount(), 0);
    QCOMPARE(proxy.columnCount(), 0);

    std::unique_ptr<QStandardItemModel> model1 = createModel(5, 3);
    proxy.setSourceModel(model1.get());

    QCOMPARE(proxy.rowCount(), 5);
    QCOMPARE(proxy.columnCount(), 1);

    std::unique_ptr<QStandardItemModel> model2 = createModel(10, 5);
    proxy.setSourceModel(model2.get());

    QCOMPARE(proxy.rowCount(), 10);
    QCOMPARE(proxy.columnCount(), 1);

    model2.reset();
}

void tst_KDTableToListProxyModel::simpleMapping()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(5, 3);
    proxy.setSourceModel(model.get());

    proxy.setRoleMapping(0, Qt::UserRole + 0, "first");
    proxy.setRoleMapping(1, Qt::UserRole + 1, "second");
    // do not map the third column, for now

    QCOMPARE(proxy.rowCount(), 5);
    QCOMPARE(proxy.columnCount(), 1);

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 2; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    QHash<int, QByteArray> roleNames{{Qt::UserRole + 0, "first"}, {Qt::UserRole + 1, "second"}};

    QCOMPARE(proxy.roleNames(), roleNames);

    // add a mapping for the third column
    proxy.setRoleMapping(2, Qt::UserRole + 2, "third");

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    roleNames.insert(Qt::UserRole + 2, "third");
    QCOMPARE(proxy.roleNames(), roleNames);

    // remove the mapping for the first column
    proxy.unsetRoleMapping(Qt::UserRole + 0);

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 1; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    roleNames.remove(Qt::UserRole + 0);
    QCOMPARE(proxy.roleNames(), roleNames);

    // make Qt::UserRole + 1 point to the third column instead of the second
    proxy.setRoleMapping(2, Qt::UserRole + 1, "second");

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 1; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(2));
        }
    }

    QCOMPARE(proxy.roleNames(), roleNames);
}

void tst_KDTableToListProxyModel::dataChanged()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(5, 4);
    proxy.setSourceModel(model.get());

    // deliberately in a random order; 4th column not mapped
    proxy.setRoleMapping(1, Qt::UserRole + 1, "second");
    proxy.setRoleMapping(0, Qt::UserRole + 0, "first");
    proxy.setRoleMapping(2, Qt::UserRole + 2, "third");

    QSignalSpy dataChangedSpy(&proxy, &QAbstractItemModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    model->item(0, 0)->setText(QStringLiteral("foo 0-0"));

    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy[0].size(), 3);
    QCOMPARE(dataChangedSpy[0][0].toModelIndex(), proxy.index(0, 0));
    QCOMPARE(dataChangedSpy[0][1].toModelIndex(), proxy.index(0, 0));
    QCOMPARE(dataChangedSpy[0][2].value<QVector<int>>(), QVector<int>{Qt::UserRole + 0});
    QCOMPARE(proxy.index(0, 0).data(Qt::UserRole + 0).toString(), QStringLiteral("foo 0-0"));

    dataChangedSpy.clear();

    model->item(0, 2)->setText(QStringLiteral("foo 0-2"));

    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy[0].size(), 3);
    QCOMPARE(dataChangedSpy[0][0].toModelIndex(), proxy.index(0, 0));
    QCOMPARE(dataChangedSpy[0][1].toModelIndex(), proxy.index(0, 0));
    QCOMPARE(dataChangedSpy[0][2].value<QVector<int>>(), QVector<int>{Qt::UserRole + 2});

    QCOMPARE(proxy.index(0, 0).data(Qt::UserRole + 2).toString(), QStringLiteral("foo 0-2"));

    dataChangedSpy.clear();

    model->item(3, 1)->setText(QStringLiteral("foo 3-1"));

    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy[0].size(), 3);
    QCOMPARE(dataChangedSpy[0][0].toModelIndex(), proxy.index(3, 0));
    QCOMPARE(dataChangedSpy[0][1].toModelIndex(), proxy.index(3, 0));
    QCOMPARE(dataChangedSpy[0][2].value<QVector<int>>(), QVector<int>{Qt::UserRole + 1});

    QCOMPARE(proxy.index(3, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("foo 3-1"));

    // modify a non-mapped role
    dataChangedSpy.clear();

    model->item(0, 0)->setData(QVariant::fromValue(QColor("red")), Qt::DecorationRole);
    QCOMPARE(dataChangedSpy.size(), 0);
    QCOMPARE(proxy.index(0, 0).data(Qt::UserRole + 0).toString(), QStringLiteral("foo 0-0"));
    QCOMPARE(proxy.index(0, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("0-1"));
    QCOMPARE(proxy.index(0, 0).data(Qt::UserRole + 2).toString(), QStringLiteral("foo 0-2"));

    // modify a non-mapped column
    dataChangedSpy.clear();

    model->item(0, 3)->setText(QStringLiteral("foo 0-3"));
    QCOMPARE(dataChangedSpy.size(), 0);

    // modify the model through the proxy
    dataChangedSpy.clear();

    QVERIFY(proxy.setData(proxy.index(0, 0), QStringLiteral("bar 0-0"), Qt::UserRole + 0));
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy[0].size(), 3);
    QCOMPARE(dataChangedSpy[0][0].toModelIndex(), proxy.index(0, 0));
    QCOMPARE(dataChangedSpy[0][1].toModelIndex(), proxy.index(0, 0));
    QCOMPARE(dataChangedSpy[0][2].value<QVector<int>>(), QVector<int>{Qt::UserRole + 0});
    QCOMPARE(proxy.index(0, 0).data(Qt::UserRole + 0).toString(), QStringLiteral("bar 0-0"));

    dataChangedSpy.clear();

    QVERIFY(proxy.setData(proxy.index(3, 0), QStringLiteral("bar 3-1"), Qt::UserRole + 1));
    QCOMPARE(dataChangedSpy.size(), 1);
    QCOMPARE(dataChangedSpy[0].size(), 3);
    QCOMPARE(dataChangedSpy[0][0].toModelIndex(), proxy.index(3, 0));
    QCOMPARE(dataChangedSpy[0][1].toModelIndex(), proxy.index(3, 0));
    QCOMPARE(dataChangedSpy[0][2].value<QVector<int>>(), QVector<int>{Qt::UserRole + 1});
    QCOMPARE(proxy.index(3, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("bar 3-1"));
}

void tst_KDTableToListProxyModel::rowManipulation()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(5, 3);
    proxy.setSourceModel(model.get());

    // deliberately in a random order
    proxy.setRoleMapping(1, Qt::UserRole + 1, "second");
    proxy.setRoleMapping(2, Qt::UserRole + 2, "third");
    proxy.setRoleMapping(0, Qt::UserRole + 0, "first");

    // model now: 0-0 | 0-1 | 0-2
    //            ... | ... | ...
    //            4-0 | 4-1 | 4-2

    QCOMPARE(proxy.rowCount(), 5);

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // insert a row at the end
    {
        QList<QStandardItem *> row;
        row << new QStandardItem(QStringLiteral("5-0")) << new QStandardItem(QStringLiteral("5-1"))
            << new QStandardItem(QStringLiteral("5-2"));
        model->appendRow(row);
    }

    // model now: 0-0 | 0-1 | 0-2
    //            ... | ... | ...
    //            5-0 | 5-1 | 5-2

    QCOMPARE(proxy.rowCount(), 6);
    for (int row = 0; row < 6; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // delete two rows
    QVERIFY(model->removeRows(1, 2));

    // model now: 0-0 | 0-1 | 0-2
    //            3-0 | 3-1 | 3-2
    //            4-0 | 4-1 | 4-2
    //            5-0 | 5-1 | 5-2

    QCOMPARE(proxy.rowCount(), 4);
    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();

            int rowInData = row;

            if (rowInData < 1)
            {
            }
            else
            {
                rowInData += 2;
            }

            QCOMPARE(data, QStringLiteral("%1-%2").arg(rowInData).arg(column));
        }
    }

    // insert two in the middle
    QVERIFY(model->insertRows(2, 2));

    {
        for (int row = 2; row < 4; ++row)
        {
            for (int column = 0; column < 3; ++column)
            {
                model->setItem(row, column, new QStandardItem(QStringLiteral("%1-%2").arg(row + 6).arg(column)));
            }
        }
    }

    // model now: 0-0 | 0-1 | 0-2
    //            3-0 | 3-1 | 3-2
    //            8-0 | 8-1 | 8-2
    //            9-0 | 9-1 | 9-2
    //            4-0 | 4-1 | 4-2
    //            5-0 | 5-1 | 5-2

    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();

            int rowInData = row;

            if (rowInData < 1)
            {
            }
            else if (rowInData < 2)
            {
                rowInData += 2;
            }
            else if (rowInData < 4)
            {
                rowInData += 6;
            }
            else
            {
            }

            QCOMPARE(data, QStringLiteral("%1-%2").arg(rowInData).arg(column));
        }
    }

    // TODO: test move rows
}

void tst_KDTableToListProxyModel::columnManipulation()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(5, 3);
    proxy.setSourceModel(model.get());

    // deliberately in a random order
    proxy.setRoleMapping(2, Qt::UserRole + 2, "third");
    proxy.setRoleMapping(0, Qt::UserRole + 0, "first");
    proxy.setRoleMapping(1, Qt::UserRole + 1, "second");

    // model now: 0-0 | 0-1 | 0-2
    //            ... | ... | ...
    //            4-0 | 4-1 | 4-2

    QCOMPARE(proxy.rowCount(), 5);

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // insert columns; mappings should be adjusted and preserved
    model->insertColumns(1, 3);
    {
        for (int row = 0; row < 5; ++row)
        {
            for (int column = 1; column < 4; ++column)
            {
                model->setItem(row, column, new QStandardItem(QStringLiteral("%1-%2").arg(row).arg(column + 6)));
            }
        }
    }

    // model now: 0-0 | 0-7 | 0-8 | 0-9 | 0-1 | 0-2
    //            ... | ... | ... | ... | ... | ...
    //            5-0 | 5-7 | 5-8 | 5-9 | 5-1 | 5-2

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // remove unmapped column
    model->removeColumns(2, 1);

    // model now: 0-0 | 0-7 | 0-9 | 0-1 | 0-2
    //            ... | ... | ... | ... | ...
    //            5-0 | 5-7 | 5-9 | 5-1 | 5-2

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // remove unmapped AND mapped column
    model->removeColumns(2, 2);

    // model now: 0-0 | 0-7 | 0-2
    //            ... | ... | ...
    //            5-0 | 5-7 | 5-2

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 2; ++column)
        {
            QModelIndex index = proxy.index(row, 0);

            int columnData = column;
            if (columnData < 1)
            {
            }
            else
            {
                columnData += 1;
            }

            QString data = index.data(Qt::UserRole + columnData).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(columnData));
        }
    }

    // add a mapping for the middle column
    proxy.setRoleMapping(1, Qt::UserRole + 7, "newColumn");

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);

            int columnData;
            switch (column)
            {
            case 0:
                columnData = 0;
                break;
            case 1:
                columnData = 7;
                break;
            case 2:
                columnData = 2;
                break;
            }

            QString data = index.data(Qt::UserRole + columnData).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(columnData));
        }
    }

    // TODO: column movement. There seems to be no model in Qt that supports it...
}

void tst_KDTableToListProxyModel::layoutManipulation()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(100, 3);
    QSortFilterProxyModel filterProxyModel;
    filterProxyModel.setSourceModel(model.get());
    proxy.setSourceModel(&filterProxyModel);

    // Setup is:
    //   QStandardItemModel -> QSortFilterProxyModel -> KDTableToListProxyModel
    // We're going to use QSortFilterProxyModel to emit layout changes,
    // and see how KDTableToListProxyModel reacts.

    proxy.setRoleMapping(0, Qt::UserRole + 0, "first");
    proxy.setRoleMapping(1, Qt::UserRole + 1, "second");
    proxy.setRoleMapping(2, Qt::UserRole + 2, "third");

    // model now:  0-0 |  0-1 |  0-2
    //             ... |  ... |  ...
    //            99-0 | 99-1 | 99-2

    // It's unfiltered and unsorted:

    QCOMPARE(proxy.rowCount(), 100);
    for (int row = 0; row < 100; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // Do a sort (i.e. vertical sort hint)
    {
        QSignalSpy layoutAboutToBeChangedSpy(&proxy, &QAbstractItemModel::layoutAboutToBeChanged);
        QVERIFY(layoutAboutToBeChangedSpy.isValid());
        QSignalSpy layoutChangedSpy(&proxy, &QAbstractItemModel::layoutChanged);
        QVERIFY(layoutChangedSpy.isValid());

        // Grab some persistent indexes in the proxy: every 4 rows (obviously in column 0)
        std::vector<QPersistentModelIndex> persistentProxyIndices;
        const int persistentProxyIndicesOffset = 4;
        for (int i = 0; i < 100; i += persistentProxyIndicesOffset)
        {
            auto index = proxy.index(i, 0);
            QVERIFY(index.isValid());
            persistentProxyIndices.push_back(index);
        }

        filterProxyModel.sort(0);

        QCOMPARE(layoutAboutToBeChangedSpy.size(), 1);
        QCOMPARE(layoutAboutToBeChangedSpy[0].size(), 2);
        QCOMPARE(layoutAboutToBeChangedSpy[0][0].value<QList<QPersistentModelIndex>>(), QList<QPersistentModelIndex>());
        QCOMPARE(layoutAboutToBeChangedSpy[0][1].value<QAbstractItemModel::LayoutChangeHint>(),
                 QAbstractItemModel::VerticalSortHint);

        QCOMPARE(layoutChangedSpy.size(), 1);
        QCOMPARE(layoutChangedSpy[0].size(), 2);
        QCOMPARE(layoutChangedSpy[0][0].value<QList<QPersistentModelIndex>>(), QList<QPersistentModelIndex>());
        QCOMPARE(layoutChangedSpy[0][1].value<QAbstractItemModel::LayoutChangeHint>(),
                 QAbstractItemModel::VerticalSortHint);

        // produce strings for 1..100, and sort them lexographically
        // (to be used as a reference for the sorting)
        QStringList rowAsStrings;
        rowAsStrings.reserve(100);
        for (int i = 0; i < 100; ++i)
        {
            rowAsStrings.push_back(QString::number(i));
        }
        std::sort(rowAsStrings.begin(), rowAsStrings.end());

        // check that the model contains the expected data
        QCOMPARE(proxy.rowCount(), 100);
        for (int row = 0; row < 100; ++row)
        {
            for (int column = 0; column < 3; ++column)
            {
                QModelIndex index = proxy.index(row, 0);
                QString data = index.data(Qt::UserRole + column).toString();
                QCOMPARE(data, QStringLiteral("%1-%2").arg(rowAsStrings.at(row)).arg(column));
            }
        }

        // check that the persistent model indices are still pointing to the right elements

        int row = 0;
        for (const auto &persistentIndex : persistentProxyIndices)
        {
            QVERIFY(persistentIndex.isValid()); // nothing got removed

            // the new row of the persistent index is the right one
            const int newRow = persistentIndex.row();
            QCOMPARE(rowAsStrings.at(newRow), QString::number(row));

            // and the data is the expected one as well
            for (int column = 0; column < 3; ++column)
            {
                QString data = persistentIndex.data(Qt::UserRole + column).toString();
                QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
            }

            row += persistentProxyIndicesOffset;
        }
    }

    // Undo the sort (no layout change hint).
    {
        QSignalSpy modelAboutToBeResetSpy(&proxy, &QAbstractItemModel::modelAboutToBeReset);
        QVERIFY(modelAboutToBeResetSpy.isValid());
        QSignalSpy modelResetSpy(&proxy, &QAbstractItemModel::modelReset);
        QVERIFY(modelResetSpy.isValid());

        filterProxyModel.sort(-1);
        filterProxyModel.invalidate();

        QCOMPARE(modelAboutToBeResetSpy.size(), 1);
        QCOMPARE(modelResetSpy.size(), 1);

        QCOMPARE(proxy.rowCount(), 100);
        for (int row = 0; row < 100; ++row)
        {
            for (int column = 0; column < 3; ++column)
            {
                QModelIndex index = proxy.index(row, 0);
                QString data = index.data(Qt::UserRole + column).toString();
                QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
            }
        }
    }
}

void tst_KDTableToListProxyModel::setData()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(5, 3);
    proxy.setSourceModel(model.get());

    // not mapping the second column
    proxy.setRoleMapping(2, Qt::UserRole + 2, "third");
    proxy.setRoleMapping(0, Qt::UserRole + 0, "first");

    for (int row = 0; row < 5; ++row)
    {
        for (int column : {0, 2})
        {
            const QModelIndex index = proxy.index(row, 0);
            const int role = Qt::UserRole + column;
            QCOMPARE(index.data(role).toString(), QStringLiteral("%1-%2").arg(row).arg(column));
            QVERIFY(proxy.setData(index, QStringLiteral("X-X"), role));
            QCOMPARE(index.data(role).toString(), QStringLiteral("X-X"));
            QCOMPARE(model->item(row, column)->data(Qt::DisplayRole).toString(), QStringLiteral("X-X"));
        }
    }
}

void tst_KDTableToListProxyModel::setItemData()
{
    KDTableToListProxyModel proxy;
    QAbstractItemModelTester tester(&proxy);
    std::unique_ptr<QStandardItemModel> model = createModel(5, 3);
    proxy.setSourceModel(model.get());

    proxy.setRoleMapping(0, Qt::UserRole + 0, "first", Qt::DisplayRole);

    // The second column gets mapped multiple times
    proxy.setRoleMapping(1, Qt::UserRole + 1, "secondA", Qt::DisplayRole);
    proxy.setRoleMapping(1, Qt::UserRole + 2, "secondB", Qt::DecorationRole);
    proxy.setRoleMapping(1, Qt::UserRole + 3, "secondC", Qt::ToolTipRole);

    // So does the third
    proxy.setRoleMapping(2, Qt::UserRole + 4, "thirdA", Qt::DisplayRole);
    proxy.setRoleMapping(2, Qt::UserRole + 5, "thirdA", Qt::UserRole);

    // Test a no-op
    QVERIFY(proxy.setItemData(proxy.index(1, 0), {}));

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column < 2; ++column)
        {
            QModelIndex index = proxy.index(row, 0);
            QString data = index.data(Qt::UserRole + column).toString();
            QCOMPARE(data, QStringLiteral("%1-%2").arg(row).arg(column));
        }
    }

    // Test roles in different columns in the source
    QVERIFY(proxy.setItemData(proxy.index(0, 0),
                              {{Qt::UserRole + 0, QStringLiteral("A")}, {Qt::UserRole + 1, QStringLiteral("B")}}));

    QCOMPARE(model->item(0, 0)->data(Qt::DisplayRole).toString(), QStringLiteral("A"));
    QCOMPARE(model->item(0, 1)->data(Qt::DisplayRole).toString(), QStringLiteral("B"));

    // Test roles in the same columns in the source
    QVERIFY(proxy.setItemData(proxy.index(1, 0), {
                                                     {Qt::UserRole + 0, QStringLiteral("A")},
                                                     {Qt::UserRole + 1, QStringLiteral("B")},
                                                     {Qt::UserRole + 2, QStringLiteral("C")},
                                                     {Qt::UserRole + 3, QStringLiteral("D")},
                                                     {Qt::UserRole + 4, QStringLiteral("E")},
                                                     {Qt::UserRole + 5, QStringLiteral("F")},
                                                 }));

    QCOMPARE(model->item(1, 0)->data(Qt::DisplayRole).toString(), QStringLiteral("A"));
    QCOMPARE(model->item(1, 1)->data(Qt::DisplayRole).toString(), QStringLiteral("B"));
    QCOMPARE(model->item(1, 1)->data(Qt::DecorationRole).toString(), QStringLiteral("C"));
    QCOMPARE(model->item(1, 1)->data(Qt::ToolTipRole).toString(), QStringLiteral("D"));
    QCOMPARE(model->item(1, 2)->data(Qt::DisplayRole).toString(), QStringLiteral("E"));
    QCOMPARE(model->item(1, 2)->data(Qt::UserRole).toString(), QStringLiteral("F"));

    QVERIFY(proxy.setItemData(proxy.index(2, 0), {
                                                     {Qt::UserRole + 0, QStringLiteral("A")},
                                                     {Qt::UserRole + 5, QStringLiteral("F")},
                                                     {Qt::UserRole + 3, QStringLiteral("D")},
                                                     {Qt::UserRole + 100, QStringLiteral("NOPE")},
                                                     {Qt::UserRole + 4, QStringLiteral("E")},
                                                     {Qt::UserRole + 2, QStringLiteral("C")},
                                                     {Qt::UserRole + 101, QStringLiteral("NOPE")},
                                                 }));

    QCOMPARE(model->item(2, 0)->data(Qt::DisplayRole).toString(), QStringLiteral("A"));
    QCOMPARE(model->item(2, 1)->data(Qt::DecorationRole).toString(), QStringLiteral("C"));
    QCOMPARE(model->item(2, 1)->data(Qt::ToolTipRole).toString(), QStringLiteral("D"));
    QCOMPARE(model->item(2, 2)->data(Qt::DisplayRole).toString(), QStringLiteral("E"));
    QCOMPARE(model->item(2, 2)->data(Qt::UserRole).toString(), QStringLiteral("F"));
}

std::unique_ptr<QStandardItemModel> tst_KDTableToListProxyModel::createModel(int rows, int columns)
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

QTEST_GUILESS_MAIN(tst_KDTableToListProxyModel)

#include "tst_kdtabletolistproxymodel.moc"

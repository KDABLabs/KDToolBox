/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "../sortproxymodel.h"
#include "vectormodel.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QString>
#include <QStringBuilder>
#include <QTest>

enum RowMovedArguments
{
    SourceParent = 0,
    SourceFromIndex = 1,
    SourceToIndex = 2,
    ToParent = 3,
    ToIndex = 4
};

enum RowsInsertedArguments
{
    InsertParent = 0,
    InsertFromIndex = 1,
    InsertToIndex = 2
};

enum DataChangedArguments
{
    TopLeftIndex = 0,
    BottomRightIndex = 1,
    ChangedRoles = 2
};

class SortProxyModelTest : public QObject
{
    Q_OBJECT

    void dumpModelContents(QAbstractItemModel &model);

public:
    using QObject::QObject;

private:
    bool verifyInternalMapping(SortProxyModel *model);

private Q_SLOTS:
    void vectorModel();
    void basics();
    void changeSingleValueRight();
    void changeSingleValueLeft();
    void changeValueContiniousRange();
    void changeValueDiscontiniousRange();
    void insertSingleValue();
    void insertMultipleContiniousValues();
    void insertMultipleDiscontiniousValues();
    void insertAtEndOfRange();
    void removeSingleValue();
    void removeMultipleContiniousValues();
    void removeMultipleDiscontiniousValues();

    void strings();
    void doubles();
    void sortOnRolesAndColumns();
};

bool SortProxyModelTest::verifyInternalMapping(SortProxyModel *model)
{
    for (unsigned long i = 0; i < model->m_proxyToSourceMap.size(); ++i)
    {
        unsigned long sourceRow = model->m_proxyToSourceMap[i];
        unsigned long proxyRow = model->m_sourceToProxyMap[sourceRow];
        if (proxyRow != i)
        {
            return false;
        }
    }
    return true;
}

template<int line, typename value_type>
void checkModelContents(const QAbstractItemModel &model, std::initializer_list<value_type> values)
{
    auto it = values.begin();

    char sizeMessage[128];
    const auto rowCount = model.rowCount();
    std::snprintf(sizeMessage, 128,
                  "unexpected model row count\n"
                  "   Actual:   %d\n"
                  "   Expected: %zd\n"
                  "Called from line %d",
                  rowCount, values.size(), line);

    QVERIFY2(rowCount == static_cast<int>(values.size()), sizeMessage);

    for (int row = 0; row < rowCount; ++row)
    {
        char contentsMessage[128];
        std::snprintf(contentsMessage, 128,
                      "unexpected model contents on row %d\n"
                      "   Actual:   %s\n"
                      "   Expected: %s\n"
                      "Called from line %d",
                      row, model.index(row, 0).data().toString().toLatin1().constData(),
                      QVariant(*it).toString().toLatin1().constData(), line);
        QVERIFY2(model.index(row, 0).data().value<value_type>() == *it++, contentsMessage);
    }
}
#define CHECKMODELCONTENTS(type) checkModelContents<__LINE__, type>

void SortProxyModelTest::vectorModel()
{
    // some basic testing of the test source model itself

    VectorModel<int> sourceModel{5, 4, 3, 2, 1};
    QCOMPARE(sourceModel.rowCount(), 5);
    CHECKMODELCONTENTS(int)(sourceModel, {5, 4, 3, 2, 1});

    // appending
    sourceModel.append(0);
    CHECKMODELCONTENTS(int)(sourceModel, {5, 4, 3, 2, 1, 0});

    sourceModel.append({-1, -2});
    CHECKMODELCONTENTS(int)(sourceModel, {5, 4, 3, 2, 1, 0, -1, -2});

    // inserting in the middle
    sourceModel.insert(1, 4);
    CHECKMODELCONTENTS(int)(sourceModel, {5, 4, 4, 3, 2, 1, 0, -1, -2});

    sourceModel.insert(3, {10, 11, 12});
    CHECKMODELCONTENTS(int)(sourceModel, {5, 4, 4, 10, 11, 12, 3, 2, 1, 0, -1, -2});

    // prepending
    sourceModel.insert(0, {42, 41});
    CHECKMODELCONTENTS(int)(sourceModel, {42, 41, 5, 4, 4, 10, 11, 12, 3, 2, 1, 0, -1, -2});

    sourceModel.insert(0, 35);
    CHECKMODELCONTENTS(int)(sourceModel, {35, 42, 41, 5, 4, 4, 10, 11, 12, 3, 2, 1, 0, -1, -2});

    // removing
    sourceModel.removeRow(0);
    CHECKMODELCONTENTS(int)(sourceModel, {42, 41, 5, 4, 4, 10, 11, 12, 3, 2, 1, 0, -1, -2});

    sourceModel.removeRows(0, 2);
    CHECKMODELCONTENTS(int)(sourceModel, {5, 4, 4, 10, 11, 12, 3, 2, 1, 0, -1, -2});

    sourceModel.removeRows(1, 2);
    CHECKMODELCONTENTS(int)(sourceModel, {5, 10, 11, 12, 3, 2, 1, 0, -1, -2});
}

void SortProxyModelTest::basics()
{
    VectorModel<int> sourceModel{5, 4, 3, 2, 1};

    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);

    QCOMPARE(sorted.columnCount(), 1);
    CHECKMODELCONTENTS(int)(sorted, {5, 4, 3, 2, 1});

    sorted.sort(0);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    sorted.sort(0, Qt::DescendingOrder);
    CHECKMODELCONTENTS(int)(sorted, {5, 4, 3, 2, 1});

    VectorModel<int> sourceModel2{3, 4, 5, 1, 2};
    sorted.setSourceModel(&sourceModel2);
    CHECKMODELCONTENTS(int)(sorted, {5, 4, 3, 2, 1});
}

void SortProxyModelTest::changeSingleValueRight()
{
    VectorModel<int> sourceModel{5, 4, 3, 2, 1};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);

    QCOMPARE(sorted.rowCount(), 5);
    sorted.sort(0);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    QSignalSpy spy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy changedSpy(&sorted, &SortProxyModel::dataChanged);

    // change single value: changed item should move right
    sourceModel.setValue(4, 6);

    // check sorted model
    CHECKMODELCONTENTS(int)(sorted, {2, 3, 4, 5, 6});

    // signal emission
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0)[SourceFromIndex], 0);
    QCOMPARE(spy.at(0)[SourceToIndex], 0);
    QCOMPARE(spy.at(0)[ToIndex], 5);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0)[TopLeftIndex].value<QModelIndex>().row(), 0);
}

void SortProxyModelTest::dumpModelContents(QAbstractItemModel &model)
{
    QStringList contents;
    contents.reserve(model.rowCount());
    for (int row = 0; row < model.rowCount(); ++row)
    {
        contents << model.index(row, 0).data().toString();
    }

    qDebug() << "model contents:" << contents.join(QLatin1String(", "));
}

void SortProxyModelTest::changeSingleValueLeft()
{
    VectorModel<int> sourceModel{5, 4, 3, 2, 1};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);

    QCOMPARE(sorted.rowCount(), 5);
    sorted.sort(0);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    QVERIFY(verifyInternalMapping(&sorted));

    QSignalSpy spy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy changedSpy(&sorted, &SortProxyModel::dataChanged);

    // change single value: changed item should move left
    const auto indexToChange = 1;
    QCOMPARE(sourceModel.index(indexToChange).data().toInt(), 4);
    sourceModel.setValue(indexToChange, -2);

    // check sorted model
    CHECKMODELCONTENTS(int)(sorted, {-2, 1, 2, 3, 5});
    // signal emission
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0)[SourceFromIndex], 0);
    QCOMPARE(spy.at(0)[SourceToIndex], 2);
    QCOMPARE(spy.at(0)[ToIndex], 4);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0)[TopLeftIndex].value<QModelIndex>().row(), 3);
    QVERIFY(verifyInternalMapping(&sorted));
}

void SortProxyModelTest::changeValueContiniousRange()
{
    VectorModel<int> sourceModel{5, 4, 3, 2, 1};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);

    sorted.sort(0);
    QCOMPARE(sorted.rowCount(), 5);
    QCOMPARE(sorted.index(0).data().toInt(), 1);
    QCOMPARE(sorted.index(4).data().toInt(), 5);
    QVERIFY(verifyInternalMapping(&sorted));

    QSignalSpy spy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy changedSpy(&sorted, &SortProxyModel::dataChanged);

    // change two rows
    sourceModel.contents[2] = 7; // changed from 3
    sourceModel.contents[3] = 6; // changed from 2
    Q_EMIT sourceModel.dataChanged(sourceModel.index(2), sourceModel.index(3));

    // check sorted model contents
    CHECKMODELCONTENTS(int)(sorted, {1, 4, 5, 6, 7});

    // signal emission
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0)[SourceFromIndex], 1);
    QCOMPARE(spy.at(0)[SourceToIndex], 2);
    QCOMPARE(spy.at(0)[ToIndex], 5);

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0)[TopLeftIndex].value<QModelIndex>().row(), 1);
    QCOMPARE(changedSpy.at(0)[BottomRightIndex].value<QModelIndex>().row(), 2);
    QVERIFY(verifyInternalMapping(&sorted));
}

void SortProxyModelTest::changeValueDiscontiniousRange()
{
    VectorModel<int> sourceModel{3, 1, 5, 2, 4};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);

    QCOMPARE(sorted.rowCount(), 5);
    sorted.sort(0);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    QSignalSpy changedSpy(&sorted, &SortProxyModel::dataChanged);
    QSignalSpy spy(&sorted, &SortProxyModel::rowsMoved);

    // change two rows, will result in two different moves in the sorted model
    sourceModel.contents[4] = 7; // changed from 4
    sourceModel.contents[1] = 6; // changed from 1
                                 // we trigger only one dataChanged signal for both changes
    Q_EMIT sourceModel.dataChanged(sourceModel.index(1), sourceModel.index(4));

    // contents before change        {1, 2, 3, 4, 5}
    //  after change, before reorder {6, 2, 3, 7, 5}
    //   1st reorder step            {6, 2, 3, 5, 7}
    //   2nd reorder step            {2, 3, 5, 6, 7}
    // check sorted model contents after reorder
    CHECKMODELCONTENTS(int)(sorted, {2, 3, 5, 6, 7});

    // signal emissions: changed signals
    QCOMPARE(changedSpy.count(), 2); // we expect two data Changed signals, though the source model emitted only one
    QCOMPARE(changedSpy.at(0)[TopLeftIndex].value<QModelIndex>().row(), 0);
    QCOMPARE(changedSpy.at(0)[BottomRightIndex].value<QModelIndex>().row(), 1);
    QCOMPARE(changedSpy.at(1)[TopLeftIndex].value<QModelIndex>().row(), 3);
    QCOMPARE(changedSpy.at(1)[BottomRightIndex].value<QModelIndex>().row(), 4);
    // row move signals
    QCOMPARE(spy.count(), 2);
    // move item 3 (7) to item 5
    QCOMPARE(spy.at(0)[SourceFromIndex], 3);
    QCOMPARE(spy.at(0)[SourceToIndex], 3);
    QCOMPARE(spy.at(0)[ToIndex], 5);
    // move item 0 (6) to item 4
    QCOMPARE(spy.at(1)[SourceFromIndex], 0);
    QCOMPARE(spy.at(1)[SourceToIndex], 0);
    QCOMPARE(spy.at(1)[ToIndex], 4);
}

void SortProxyModelTest::insertSingleValue()
{
    VectorModel<int> sourceModel{3, 1, 5, 4};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);

    sorted.sort(0);
    CHECKMODELCONTENTS(int)(sorted, {1, 3, 4, 5});
    QVERIFY(verifyInternalMapping(&sorted));

    QSignalSpy movedSpy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy insertedSpy(&sorted, &SortProxyModel::rowsInserted);

    sourceModel.append(2);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});
    QCOMPARE(movedSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 1);

    sourceModel.insert(2, 2);
    CHECKMODELCONTENTS(int)(sourceModel, {3, 1, 2, 5, 4, 2});
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 2, 3, 4, 5});
    QCOMPARE(movedSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 2);
    QVERIFY(verifyInternalMapping(&sorted));
}

void SortProxyModelTest::insertMultipleContiniousValues()
{
    VectorModel<int> sourceModel{3, 9, 1, 2, 4};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 9});

    QSignalSpy movedSpy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy insertedSpy(&sorted, &SortProxyModel::rowsInserted);

    sourceModel.append({6, 7, 8});

    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 6, 7, 8, 9});
    QCOMPARE(movedSpy.count(), 0);    // no moves, just inserts
    QCOMPARE(insertedSpy.count(), 1); // we expect the insert to be handled in one single insert
    QCOMPARE(insertedSpy.at(0)[InsertFromIndex], 4);
    QCOMPARE(insertedSpy.at(0)[InsertToIndex], 6);
    QVERIFY(verifyInternalMapping(&sorted));
}

void SortProxyModelTest::insertMultipleDiscontiniousValues()
{
    VectorModel<int> sourceModel{3, 9, 1, 2, 4};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 9});

    QSignalSpy movedSpy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy insertedSpy(&sorted, &SortProxyModel::rowsInserted);

    sourceModel.append({8, 3, -2});

    CHECKMODELCONTENTS(int)(sorted, {-2, 1, 2, 3, 3, 4, 8, 9});
    QCOMPARE(movedSpy.count(), 0); // no moves, just inserts
    // we expect the source insert to be handled in three distinct insert in the sorted model
    QCOMPARE(insertedSpy.count(), 3);
    QCOMPARE(insertedSpy.at(0)[InsertFromIndex], 0);
    QCOMPARE(insertedSpy.at(0)[InsertToIndex], 0);
    QCOMPARE(insertedSpy.at(1)[InsertFromIndex], 4);
    QCOMPARE(insertedSpy.at(1)[InsertToIndex], 4);
    QCOMPARE(insertedSpy.at(2)[InsertFromIndex], 6);
    QCOMPARE(insertedSpy.at(2)[InsertToIndex], 6);

    QVERIFY(verifyInternalMapping(&sorted));
}

void SortProxyModelTest::insertAtEndOfRange()
{
    VectorModel<int> sourceModel{};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    QCOMPARE(sorted.rowCount(), 0);
    QSignalSpy insertedSpy(&sorted, &SortProxyModel::rowsInserted);

    // append to empty model
    sourceModel.append({3, 1, 2});
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3});
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0)[InsertFromIndex], 0);
    QCOMPARE(insertedSpy.at(0)[InsertToIndex], 2);

    // append to non-empty model
    insertedSpy.clear();
    sourceModel.append({4, 42});
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 42});
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0)[InsertFromIndex], 3);
    QCOMPARE(insertedSpy.at(0)[InsertToIndex], 4);

    // insert into non-empty model
    insertedSpy.clear();
    sourceModel.insert(2, 75);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 42, 75});
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0)[InsertFromIndex], 5);
    QCOMPARE(insertedSpy.at(0)[InsertToIndex], 5);
}

void SortProxyModelTest::removeSingleValue()
{
    VectorModel<int> sourceModel{3, 9, 1, 2, 4};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 9});

    QSignalSpy movedSpy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy removedSpy(&sorted, &SortProxyModel::rowsRemoved);

    sourceModel.removeRow(1);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4});
    QCOMPARE(movedSpy.count(), 0); // no moves, just removes
    QCOMPARE(removedSpy.count(), 1);
}

void SortProxyModelTest::removeMultipleContiniousValues()
{
    VectorModel<int> sourceModel{5, 4, 3, 2, 1};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    QSignalSpy movedSpy(&sorted, &SortProxyModel::rowsMoved);
    QSignalSpy removedSpy(&sorted, &SortProxyModel::rowsRemoved);

    sourceModel.removeRows(1, 3);
    CHECKMODELCONTENTS(int)(sorted, {1, 5});
    QCOMPARE(movedSpy.count(), 0); // no moves, just removes
    QCOMPARE(removedSpy.count(), 1);
}

void SortProxyModelTest::removeMultipleDiscontiniousValues()
{
    VectorModel<int> sourceModel{3, 5, 1, 2, 4};
    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    auto sortedPtr = &sorted;
    int removal = 0;
    QSignalSpy movedSpy(sortedPtr, &SortProxyModel::rowsMoved);
    QSignalSpy removedSpy(sortedPtr, &SortProxyModel::rowsRemoved);

    // We expect to temporariy have some invalid content in the model during the removal operation.
    // These rows will return invalid QVariants
    connect(&sorted, &SortProxyModel::rowsRemoved, &sorted, [&] {
        ++removal;
        switch (removal)
        {
        case 1:
            CHECKMODELCONTENTS(QVariant)(*sortedPtr, {QVariant(), QVariant(), QVariant(3), QVariant(4)});
            break;
        case 2:
            CHECKMODELCONTENTS(int)(*sortedPtr, {3, 4});
            break;
        default:
            QFAIL("This point should not have been reached.");
        }
    });

    sourceModel.removeRows(1, 3);
    CHECKMODELCONTENTS(int)(sorted, {3, 4});
    QCOMPARE(movedSpy.count(), 0); // no moves, just removes
    QCOMPARE(removedSpy.count(), 2);
}

void SortProxyModelTest::strings()
{
    VectorModel<QString> sourceModel{QLatin1String("cherry"), QLatin1String("dew"), QLatin1String("Bee"),
                                     QLatin1String("Echo"), QLatin1String("apple")};

    SortProxyModel sorted;
    sorted.setSortCaseSensitivity(Qt::CaseInsensitive);
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    CHECKMODELCONTENTS(QString)
    (sorted, {QLatin1String("apple"), QLatin1String("Bee"), QLatin1String("cherry"), QLatin1String("dew"),
              QLatin1String("Echo")});
    sorted.setSortCaseSensitivity(Qt::CaseSensitive);

    CHECKMODELCONTENTS(QString)
    (sorted, {QLatin1String("Bee"), QLatin1String("Echo"), QLatin1String("apple"), QLatin1String("cherry"),
              QLatin1String("dew")});

    sorted.sort(0, Qt::DescendingOrder);
    CHECKMODELCONTENTS(QString)
    (sorted, {QLatin1String("dew"), QLatin1String("cherry"), QLatin1String("apple"), QLatin1String("Echo"),
              QLatin1String("Bee")});
}

void SortProxyModelTest::doubles()
{
    VectorModel<double> sourceModel{20.0, 1.1, 42.0, 3.33};

    SortProxyModel sorted;
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0);

    CHECKMODELCONTENTS(double)(sorted, {1.1, 3.33, 20.0, 42.0});

    sorted.sort(0, Qt::DescendingOrder);
    CHECKMODELCONTENTS(double)(sorted, {42.0, 20.0, 3.33, 1.1});
}

void SortProxyModelTest::sortOnRolesAndColumns()
{
    QStandardItemModel sourceModel;
    struct Entry
    {
        int display;
        int edit;
    };

    std::vector<Entry> data = {{1, 5}, {2, 4}, {3, 3}, {4, 2}, {5, 1}};

    sourceModel.setColumnCount(2);
    sourceModel.setRowCount(5);
    int row = 0;
    for (const auto &entry : data)
    {
        auto item = new QStandardItem();
        item->setData(entry.display, Qt::DisplayRole);
        item->setData(entry.edit, Qt::UserRole);

        sourceModel.setItem(row, 0, item);
        auto col2Item = new QStandardItem();
        col2Item->setData(entry.edit, Qt::DisplayRole);
        col2Item->setData(entry.display, Qt::UserRole);

        sourceModel.setItem(row, 1, col2Item);
        row++;
    }

    SortProxyModel sorted;
    sorted.setSortRole(Qt::UserRole);
    sorted.setSourceModel(&sourceModel);
    sorted.sort(0, Qt::AscendingOrder);

    // sort on Qt::UserRole in column 0
    CHECKMODELCONTENTS(int)(sorted, {5, 4, 3, 2, 1});

    // sort on Qt::UserRole in column 1
    sorted.sort(1, Qt::AscendingOrder);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});

    // sort on Qt::DisplayRole in column 2
    sorted.setSortRole(Qt::DisplayRole);
    CHECKMODELCONTENTS(int)(sorted, {5, 4, 3, 2, 1});

    // remove sorting
    sorted.sort(-1);
    CHECKMODELCONTENTS(int)(sorted, {1, 2, 3, 4, 5});
}

QTEST_MAIN(SortProxyModelTest)

#include "tst_sortproxymodeltest.moc"

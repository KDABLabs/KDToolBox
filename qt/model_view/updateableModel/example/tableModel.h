#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QAbstractListModel>
#include "../UpdateableModel.h"
#include "Data.h"
#include <vector>
#include <list>
#include <QList>

class TableModel : public UpdateableModel<QAbstractTableModel, Data>
{
public:
    TableModel(QObject* parent = nullptr);

    Q_SLOT void update(const DataContainer& updatedData);

protected:    //QAIM API
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    //should work with any random-access container with index-based access:

//    DataContainer m_data; // QVector, that is
//    std::vector<Data> m_data;
    QList<Data> m_data;
};

#endif // TABLEMODEL_H

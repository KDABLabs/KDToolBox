/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include "../UpdateableModel.h"
#include "Data.h"
#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QList>
#include <list>
#include <vector>

class TableModel : public UpdateableModel<QAbstractTableModel, Data>
{
    Q_OBJECT
public:
    explicit TableModel(QObject *parent = nullptr);
    ~TableModel() override;

    Q_SLOT void update(const DataContainer &updatedData);

protected: // QAIM API
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    // should work with any random-access container with index-based access:

    //    QVector<Data> m_data;  // if you use QVector, you also need to enable the default constructor of Data in
    //    Data.h
    std::vector<Data> m_data;
    //    QList<Data> m_data;
};

#endif // TABLEMODEL_H

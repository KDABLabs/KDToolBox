/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <KDFunctionalSortFilterProxyModel.h>

#include <QtWidgets>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("Colors with long names"));
    QApplication app(argc, argv);

    QStandardItemModel model;
    const auto colorNames = QColor::colorNames();
    for (const QString &name : colorNames)
    {
        auto *item = new QStandardItem;
        item->setData(QVariant::fromValue(name), Qt::DisplayRole);
        item->setData(QVariant::fromValue(QColor(name)), Qt::DecorationRole);
        model.appendRow(item);
    }

    KDFunctionalSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    auto colorIsLong = [](const QAbstractItemModel *model, int source_row, const QModelIndex &parent) {
        auto index = model->index(source_row, 0, parent);
        return index.data(Qt::DisplayRole).toString().size() >= 10;
    };

    proxy.setFilterAcceptsRowFunction(colorIsLong);

    QListView view;
    view.setModel(&proxy);
    view.show();

    return app.exec();
}

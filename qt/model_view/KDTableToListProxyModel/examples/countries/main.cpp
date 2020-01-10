/****************************************************************************
**                                MIT License
**
** Copyright (C) 2019-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>

#include <QTimer>

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <KDTableToListProxyModel.h>

#include <memory>
#include <chrono>

struct Country {
    const char *name;
    double population;
    const char *flag;
} countries[] = {
    { "Austria", 8.858, "images/Austria.png" },
    { "Belgium", 11.467, "images/Belgium.png" },
    { "Bulgaria", 7.0, "images/Bulgaria.png" },
    { "Croatia", 4.076, "images/Croatia.png" },
    { "Cyprus", 0.875, "images/Cyprus.png" },
    { "Czech Republic", 10.649, "images/Czech Republic.png" },
    { "Denmark", 5.806, "images/Denmark.png" },
    { "Estonia", 1.324, "images/Estonia.png" },
    { "Finland", 5.517, "images/Finland.png" },
    { "France", 67.028, "images/France.png" },
    { "Germany", 83.019, "images/Germany.png" },
    { "Greece", 10.722, "images/Greece.png" },
    { "Hungary", 9.797, "images/Hungary.png" },
    { "Ireland", 4.904, "images/Ireland.png" },
    { "Italy", 60.359, "images/Italy.png" },
    { "Latvia", 1.919, "images/Latvia.png" },
    { "Lithuania", 2.794, "images/Lithuania.png" },
    { "Luxembourg", 0.613, "images/Luxembourg.png" },
    { "Malta", 0.493, "images/Malta.png" },
    { "Netherlands", 12.282, "images/Netherlands.png" },
    { "Poland", 37.972, "images/Poland.png" },
    { "Portugal", 10.276, "images/Portugal.png" },
    { "Romania", 19.401, "images/Romania.png" },
    { "Slovakia", 5.45, "images/Slovakia.png" },
    { "Slovenia", 2.080, "images/Slovenia.png" },
    { "Spain", 46.934, "images/Spain.png" },
    { "Sweden", 10.230, "images/Sweden.png" },
    { "United Kingdom", 66.647, "images/United Kingdom.png" },
};

std::unique_ptr<QStandardItemModel> createCountriesModel()
{
    std::unique_ptr<QStandardItemModel> countriesModel(new QStandardItemModel);

    for (const auto &country : countries) {
        QList<QStandardItem *> row;
        row << new QStandardItem(country.name);

        auto populationItem = new QStandardItem;
        populationItem->setData(country.population, Qt::DisplayRole);
        row << populationItem;

        row << new QStandardItem(country.flag);

        countriesModel->appendRow(row);
    }

    return countriesModel;
}

static const auto POPULATION_INCREASE_INTERVAL = std::chrono::seconds(3);

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    // Source model. For simplicity it's a QStandardItemModel,
    // arranged as a table
    std::unique_ptr<QStandardItemModel> countriesModel = createCountriesModel();

    // Filter on the source model
    QSortFilterProxyModel filterProxyModel;
    filterProxyModel.setFilterKeyColumn(0);
    filterProxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    filterProxyModel.setSourceModel(countriesModel.get());

    // Table to list proxy, on the filter's output. Map the columns
    // of the source to arbitrary roles, all in column 0. For Qt Quick
    // purposes, also give symbolic names to those roles
    KDTableToListProxyModel tableToListProxyModel;
    tableToListProxyModel.setSourceModel(&filterProxyModel);

    tableToListProxyModel.setRoleMapping(0, Qt::UserRole + 0, "name");
    tableToListProxyModel.setRoleMapping(1, Qt::UserRole + 1, "population");
    tableToListProxyModel.setRoleMapping(2, Qt::UserRole + 2, "flag");


    // Load and show some Qt Quick content
    QQuickView view;
    view.setTitle("Countries of the European Union");
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.engine()->rootContext()->setContextProperty(QStringLiteral("_filter"), &filterProxyModel);
    view.engine()->rootContext()->setContextProperty(QStringLiteral("_model"), &tableToListProxyModel);
    view.setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    view.show();

    // Have the source model change
    QTimer populationIncreaseTimer;
    populationIncreaseTimer.callOnTimeout([&]()
    {
        for (int row = 0; row < countriesModel->rowCount(); ++row) {
            auto item = countriesModel->item(row, 1);
            auto currentPopulation = item->data(Qt::DisplayRole).toDouble();
            currentPopulation *= 1.01;
            item->setData(currentPopulation, Qt::DisplayRole);
        }
    });
    populationIncreaseTimer.start(POPULATION_INCREASE_INTERVAL);

    return app.exec();
}

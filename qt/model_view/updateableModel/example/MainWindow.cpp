/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include "MainWindow.h"
#include "Data.h"
#include "tableModel.h"
#include "ui_MainWindow.h"
#include <QTime>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new TableModel(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);

    DataContainer data{
        {0, QStringLiteral("0"), QTime::currentTime().toString()},
        {1, QStringLiteral("1"), QTime::currentTime().toString()},
        {2, QStringLiteral("2"), QTime::currentTime().toString()},
    };
    m_model->update(data);
    m_data = data;

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateData);

    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateData()
{
    // update the data once every second. We update some items, we sometimes add an item and sometimes remove one.
    auto curTime = QTime::currentTime();

    // every 5 seconds, either add or remove an item
    if (curTime.second() % 5 == 0)
    {
        auto itAt1 = std::next(m_data.begin(), 1);
        if (m_data.size() == 3)
        {
            m_data.erase(itAt1);
        }
        else
        {
            m_data.insert(itAt1, Data{1, QStringLiteral("1"), QTime::currentTime().toString()});
        }
    }
    else
    { // otherwise, update the time for every other item.
        for (size_t i = 0; i < m_data.size(); ++i)
        {
            if ((i % 2) == static_cast<size_t>((curTime.second() % 2)))
            {
                Data &item = m_data[i];
                item.value2 = QTime::currentTime().toString();
            }
        }
    }
    m_model->update(m_data);
}

/****************************************************************************
**                                MIT License
**
** Copyright (C) 2018-2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: André Somers <andre.somers@kdab.com>
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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "tableModel.h"
#include "Data.h"
#include <QTime>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_model(new TableModel(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);

    DataContainer data {
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
    //update the data once every second. We update some items, we sometimes add an item and sometimes remove one.
    auto curTime = QTime::currentTime();

    //every 5 seconds, either add or remove an item
    if (curTime.second()%5 == 0) {
        auto itAt1 = std::next(m_data.begin(), 1);
        if (m_data.size() == 3) {
            m_data.erase(itAt1);
        } else {
            m_data.insert(itAt1, Data{1, QStringLiteral("1"), QTime::currentTime().toString()});
        }
    } else { //otherwise, update the time for every other item.
        for (size_t i=0; i<m_data.size(); ++i) {
            if ((i % 2) == static_cast<size_t>((curTime.second() % 2))) {
                Data& item = m_data[i];
                item.value2 = QTime::currentTime().toString();
            }
        }
    }
    m_model->update(m_data);
}


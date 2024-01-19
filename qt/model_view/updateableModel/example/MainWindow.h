/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: André Somers <andre.somers@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Data.h"
#include <QMainWindow>

class TableModel;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    // make some updates to the data
    void updateData();

private:
    Ui::MainWindow *ui;

    // the model inheriting from UpdateableModel
    TableModel *m_model;

    // the data that we are changing and then pushing to the model
    DataContainer m_data;
};

#endif // MAINWINDOW_H

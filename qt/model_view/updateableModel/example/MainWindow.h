#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Data.h"

class TableModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    //make some updates to the data
    void updateData();

private:
    Ui::MainWindow *ui;

    //the model inheriting from UpdateableModel
    TableModel* m_model;

    //the data that we are changing and then pushing to the model
    DataContainer m_data;
};

#endif // MAINWINDOW_H

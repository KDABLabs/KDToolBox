#ifndef DATA_H
#define DATA_H
#include <QString>
#include <QVector>
#include <vector>

struct Data {
    int id;
    QString value1;
    QString value2;
};

//using DataContainer = QVector<Data>;
using DataContainer = std::vector<Data>;

#endif // DATA_H

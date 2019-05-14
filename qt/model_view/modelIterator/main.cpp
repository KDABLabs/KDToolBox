#include <QCoreApplication>
#include "ModelIterator.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <QModelIndex>
#include <algorithm>

template<typename T>
void addSubtree(T *node, int depth, int count, QString text)
{
    for (int i(0);i<count; ++i) {
        QString itemText = text + "." + QString::number(i);
        auto item = new QStandardItem(itemText);
        if (depth > 1) {
            addSubtree(item, depth - 1, count, itemText);
        }
        node->appendRow(item);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStandardItemModel model;

    //build up test tree
    addSubtree(&model, 3, 5, QString("node "));

    qDebug() << "root row count" << model.rowCount();

    qDebug() << "flat iterator, linear scan:";
    auto adapter = ModelAdapter<FlatIterator>(&model);
    for(const auto& index: adapter) {
        qDebug() << index.data() << index;
    }

    qDebug() << "There are" << adapter.end() - adapter.begin() << "nodes in the adapter";

    qDebug() << "\n\nDepth-first iterator, linear scan:";
    auto adapter2 = ModelAdapter<DepthFirstIterator>(&model);
    for(const auto& index: adapter2) {
        qDebug() << index.data() << index;
    }

    qDebug() << "\n\nDepth-first iterator, std::find_if:";
    auto it = std::find_if(adapter2.begin(), adapter2.end(), [](const QModelIndex& index)
    {
        return index.data().toString() == QStringLiteral("node .3.2.4");
    });
    if (it == adapter2.end()) {
        qDebug() << "Node not found!";
    } else {
        qDebug() << "Node found:" << *it << it->data() << "\n\n";
    }

    auto it2 = DataValueWrapper<DepthFirstIterator, QString>::begin(&model);
    const auto end2 = DataValueWrapper<DepthFirstIterator, QString>::end(&model);

    qDebug() << "\n\nDepth-first iterator, std::find_if on DataValueWrapped iterators:";
    auto findit = std::find_if(it2, end2, [](const QString& label)
    {
        return label == QStringLiteral("node .3.2.4");
    });
    if (findit == end2) {
        qDebug() << "Node not found!";
    } else {
        qDebug() << "Node found:" << *findit << findit.index() << "\n\n";
    }

    //Assign to DatavalueWrapper iterator. This will call setData on the model
    while (it2 != end2) {
        *it2=QStringLiteral("visited");
        ++it2;
    }

    //We can use ModelAdapter with an additional template argument to iterate over that type
    qDebug() << "\nvisited all top nodes?";
    auto adapter3 = ModelAdapter<FlatIterator, QString>(&model);
    for (const QString &label: adapter3) {
        qDebug() << label;
    }

    //And that adapter works with standard algorithms too
    if (std::all_of(adapter3.begin(), adapter3.end(), [](const QString& label) {
                    return label == QStringLiteral("visited");
    })) {
        qDebug() << "yes, all visited";
    } else {
        qDebug() << "no, not all visited!";
    }

    return 0;
}

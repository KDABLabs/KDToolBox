#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>

class Controller : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void crash() {
        delete this; delete this;
    }
};


int main(int a, char **b)
{
    QGuiApplication app(a, b);

    QQuickView view;
    view.rootContext()->setContextProperty("_controller", new Controller());
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}

#include <main.moc>

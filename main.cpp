#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "OICServer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    OICServer* server = new OICServer();
    server->start();



    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

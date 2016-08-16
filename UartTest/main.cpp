#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "uart.h"
#include "proto1.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    uart* hwlayer = new uart;
    proto1* protohandler = new proto1(hwlayer);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    context->setContextProperty("uart", hwlayer);
    context->setContextProperty("comm", protohandler);

//    context->setContextProperty("comm", comm);
//    context->setContextProperty("board", board);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

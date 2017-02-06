#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "uart.h"
#include "proto1.h"
#include "guiglue.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    proto1* protohandler = new proto1();
    guiglue* guihandler = new guiglue(protohandler);

    context->setContextProperty("comm", protohandler);
    context->setContextProperty("guiglue", guihandler);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    int ret = app.exec();
    delete protohandler;
    delete guihandler;

    return ret;

}


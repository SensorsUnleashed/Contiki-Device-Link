#include "database.h"
#include "coaphandler.h"
#include "sensorsunleashed.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QVariant>

int main(int argc, char *argv[])
{
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    database* db = new database();
    coaphandler* coap = new coaphandler(db);
    sensorsunleashed* su = new sensorsunleashed(db, coap);

    context->setContextProperty("coap", coap);
    context->setContextProperty("su", su);

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
}



//#include <QApplication>
//#include <QQmlApplicationEngine>
//#include <QQmlContext>
//#include "uart.h"
//#include "proto1.h"
//#include "guiglue.h"

//int main(int argc, char *argv[])
//{
//    QApplication app(argc, argv);

//    QQmlApplicationEngine engine;
//    QQmlContext *context = engine.rootContext();

//    proto1* protohandler = new proto1();
//    guiglue* guihandler = new guiglue(protohandler);

//    context->setContextProperty("comm", protohandler);
//    context->setContextProperty("guiglue", guihandler);

//    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

//    return app.exec();
//}
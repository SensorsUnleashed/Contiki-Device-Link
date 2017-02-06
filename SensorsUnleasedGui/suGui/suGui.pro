TEMPLATE = app
QT += qml quick widgets network sql

CONFIG += c++11

SOURCES += main.cpp \
    cantcoap/cantcoap.cpp \
    coaphandler.cpp \
    sensorsunleashed.cpp \
    database.cpp \
    ../../apps/uartsensors/cmp.c \
    ../../contiki/core/lib/crc16.c \
    ../../apps/uartsensors/uart_protocolhandler.c \
    node.cpp \
    wsn.cpp \
    socket.cpp \
    helper.cpp \
    sensor.cpp \
    powerrelay.cpp \
    pulsecounter.cpp \
    ledindicator.cpp \
    pairlist.cpp \
    sensorstore.cpp

RESOURCES += \
    pages.qrc \
    widgets.qrc \
    qml.qrc \
    sensorwidgets.qrc \
    nodewidgets.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    Database/setup.db

HEADERS += \
    cantcoap/cantcoap.h \
    cantcoap/dbg.h \
    coaphandler.h \
    sensorsunleashed.h \
    cantcoap/sysdep.h \
    database.h \
    ../../apps/uartsensors/cmp.h \
    ../../contiki/core/lib/crc16.h \
    node.h \
    wsn.h \
    socket.h \
    coapheaders.h \
    helper.h \
    pairlist.h \
    sensorstore.h

copydata.commands = $(COPY_DIR) $$PWD/Database/setup.db $$OUT_PWD/Database/setup.db
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

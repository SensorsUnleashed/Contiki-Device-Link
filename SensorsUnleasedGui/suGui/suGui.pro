TEMPLATE = app
QT += qml quick widgets network sql

CONFIG += c++11

SOURCES += main.cpp \
    cantcoap/cantcoap.cpp \
    coaphandler.cpp \
    sensorsunleashed.cpp \
    database.cpp \
    ../../sensorsUnleashed/lib/cmp.c \
    ../../contiki/core/lib/crc16.c \
    ../../sensorsUnleashed/lib/cmp_helpers.c \
    node.cpp \
    wsn.cpp \
    socket.cpp \
    helper.cpp \
    sensor.cpp \
    powerrelay.cpp \
    pulsecounter.cpp \
    pairlist.cpp \
    sensorstore.cpp \
    sumessage.cpp \
    suinterface.cpp \
    borderrouter.cpp

RESOURCES += \
    pages.qrc \
    widgets.qrc \
    qml.qrc \
    sensorwidgets.qrc \
    nodewidgets.qrc \
    pairwidgets.qrc

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
    ../../sensorsUnleashed/lib/cmp.h \
    ../../contiki/core/lib/crc16.h \
    node.h \
    wsn.h \
    socket.h \
    coapheaders.h \
    helper.h \
    pairlist.h \
    sensorstore.h \
    sumessage.h \
    borderrouter.h

copydata.commands = $(COPY_DIR) $$PWD/Database/setup.db $$OUT_PWD/Database/setup.db
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

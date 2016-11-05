TEMPLATE = app
QT += qml quick widgets network sql

CONFIG += c++11

SOURCES += main.cpp \
    cantcoap/cantcoap.cpp \
    coaphandler.cpp \
    sensorsunleashed.cpp \
    cmp/cmp.c \
    database.cpp

RESOURCES += \
    pages.qrc \
    widgets.qrc \
    qml.qrc

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
    cmp/cmp.h \
    database.h

copydata.commands = $(COPY_DIR) $$PWD/Database/setup.db $$OUT_PWD/Database/setup.db
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

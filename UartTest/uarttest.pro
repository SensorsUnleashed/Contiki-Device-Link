TEMPLATE = app

QT += qml quick widgets
QT += serialport

SOURCES += main.cpp \
    uart.cpp \
    helper.cpp \
    proto1.cpp \
    ../contiki/core/lib/crc16.c

RESOURCES += qml.qrc \
    widgets.qrc \
    screens.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    helper.h \
    proto1.h \
    ../contiki/core/lib/crc16.h \
    uart.h


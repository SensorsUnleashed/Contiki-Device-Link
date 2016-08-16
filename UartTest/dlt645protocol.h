#ifndef DLT645PROTOCOL_H
#define DLT645PROTOCOL_H
#include <QObject>
#include <QVariant>
#include "uart.h"

#define SERIAL_CALIBRATION_PASSWORD_1               0x1234
#define SERIAL_CALIBRATION_PASSWORD_2               0x5678
#define SERIAL_CALIBRATION_PASSWORD_3               0x9ABC
#define SERIAL_CALIBRATION_PASSWORD_4               0xDEF0

class dlt645protocol : public QObject
{
    Q_OBJECT

public:
    dlt645protocol(uart *comm);

    Q_INVOKABLE QStringList getCommandsList();
    Q_INVOKABLE QString get_readings(QString key);
private:
    QVariantMap commands;
    uart* comm;

    QVariantMap readings;

    int parseMessage(QByteArray &msg);

private slots:
    void handleIncomming();

signals:
    void get_meter_configuration_rdy();
    void get_readings_phase_1_rdy();

public slots:
    void execCommand(QString command);

};

enum host_commands_e
{

    HOST_CMD_SET_AUTO_REPORT_MODE               = 0x50,
    HOST_CMD_SET_POLLING_MODE                   = 0x51,
    HOST_CMD_GET_METER_NAME                     = 0x52,
    HOST_CMD_GET_METER_VER                      = 0x53,

    HOST_CMD_GET_METER_CONFIGURATION            = 0x56,
    HOST_CMD_SET_METER_CONSUMPTION              = 0x57,
    HOST_CMD_SET_RTC                            = 0x58,
    HOST_CMD_GET_RTC                            = 0x59,
    HOST_CMD_ALIGN_WITH_CALIBRATION_FACTORS     = 0x5A,
    HOST_CMD_SET_PASSWORD                       = 0x60,
    HOST_CMD_GET_READINGS_PHASE_1               = 0x61,
    HOST_CMD_GET_READINGS_PHASE_2               = 0x62,
    HOST_CMD_GET_READINGS_PHASE_3               = 0x63,
    HOST_CMD_GET_READINGS_NEUTRAL               = 0x64,
    HOST_CMD_GET_CONSUMPTION_PHASE_1            = 0x65,
    HOST_CMD_GET_CONSUMPTION_PHASE_2            = 0x66,
    HOST_CMD_GET_CONSUMPTION_PHASE_3            = 0x67,
    HOST_CMD_GET_CONSUMPTION_AGGREGATE          = 0x68,
    HOST_CMD_GET_EXTRA_READINGS_PHASE_1         = 0x69,
    HOST_CMD_GET_EXTRA_READINGS_PHASE_2         = 0x6A,
    HOST_CMD_GET_EXTRA_READINGS_PHASE_3         = 0x6B,
    HOST_CMD_GET_EXTRA_READINGS_NEUTRAL         = 0x6C,
    HOST_CMD_GET_READINGS_AGGREGATE             = 0x6D,
    HOST_CMD_GET_EXTRA_READINGS_AGGREGATE       = 0x6E,
    HOST_CMD_SUMCHECK_MEMORY                    = 0x75,
    HOST_CMD_GET_RAW_ACTIVE_POWER_PHASE_1       = 0x91,
    HOST_CMD_GET_RAW_ACTIVE_POWER_PHASE_2       = 0x92,
    HOST_CMD_GET_RAW_ACTIVE_POWER_PHASE_3       = 0x93,
    HOST_CMD_GET_RAW_REACTIVE_POWER_PHASE_1     = 0x95,
    HOST_CMD_GET_RAW_REACTIVE_POWER_PHASE_2     = 0x96,
    HOST_CMD_GET_RAW_REACTIVE_POWER_PHASE_3     = 0x97,
    HOST_CMD_GET_RAW_ACTIVE_POWER_NEUTRAL       = 0x99,
    HOST_CMD_GET_RAW_REACTIVE_POWER_NEUTRAL     = 0x9D,
    HOST_CMD_CHECK_RTC_ERROR                    = 0xA0,
    HOST_CMD_RTC_CORRECTION                     = 0xA1,
    HOST_CMD_MULTIRATE_SET_PARAMETERS           = 0xC0,
    HOST_CMD_MULTIRATE_GET_PARAMETERS           = 0xC1,
    HOST_CMD_MULTIRATE_CLEAR_USAGE              = 0xC2,
    HOST_CMD_MULTIRATE_GET_USAGE                = 0xC3,
    HOST_CMD_CLEAR_CALIBRATION_DATA             = 0xD0,
    HOST_CMD_SET_CALIBRATION_PHASE_1            = 0xD1,
    HOST_CMD_SET_CALIBRATION_PHASE_2            = 0xD2,
    HOST_CMD_SET_CALIBRATION_PHASE_3            = 0xD3,
    HOST_CMD_SET_CALIBRATION_NEUTRAL            = 0xD4,
    HOST_CMD_SET_CALIBRATION_EXTRAS             = 0xD5,
    HOST_CMD_GET_CALIBRATION_PHASE_1            = 0xD6,
    HOST_CMD_GET_CALIBRATION_PHASE_2            = 0xD7,
    HOST_CMD_GET_CALIBRATION_PHASE_3            = 0xD8,
    HOST_CMD_GET_CALIBRATION_NEUTRAL            = 0xD9,
    HOST_CMD_GET_CALIBRATION_EXTRAS             = 0xDA,
    HOST_CMD_UNKNOWN
};

#endif // DLT645PROTOCOL_H

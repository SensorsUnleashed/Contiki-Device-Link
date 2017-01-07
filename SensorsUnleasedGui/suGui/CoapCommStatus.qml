import QtQuick 2.0
import QtQuick.Controls 2.0

Column{
    property var deviceptr;
    spacing: 10;

    //        BusyIndicator{
    //            height: 30;
    //        }

    ProgressBar {
        id: timeoutcount
        value: 0;
        from: 0;
        to: 1;
    }

    Connections{
        target: deviceptr;
        onTimeoutinfo:{
            timeoutcount.to = maxretries;
            timeoutcount.value = retransnumber;
        }
    }
}


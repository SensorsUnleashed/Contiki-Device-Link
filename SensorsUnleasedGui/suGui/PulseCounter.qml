import QtQuick 2.0
import QtQuick.Layouts 1.3

RowLayout{
    spacing: 10;
    SUButton{
        property bool polling: false;
        text: qsTr("Start 5s poll");
        width: 150;
        onClicked: {
            if(!polling){
                text = "Stop poll";
                activeSensor.startPoll(5);
                polling = true;
            }
            else{
                polling = false;
                text = "Start 5s poll";
                activeSensor.startPoll(0);
            }
        }
        Component.onDestruction: activeSensor.startPoll(0);
    }
}




import QtQuick 2.0

Rectangle {
    id: sensorinfoscreen;
    color: suPalette.window;

    border.color: suPalette.buttonText;

    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database

    anchors.margins: 20;
    anchors.centerIn: parent;

    Column{
        anchors.fill: parent;
        anchors.margins: 20;
        SensorInformation {
            nodeaddr: sensorinfoscreen.nodeaddr;
            nodeinfo: sensorinfoscreen.nodeinfo;
        }
        Row{
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
    }
}




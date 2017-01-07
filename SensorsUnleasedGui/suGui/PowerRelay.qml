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
        spacing: 10;

        SensorInformation {
            nodeaddr: sensorinfoscreen.nodeaddr;
            nodeinfo: sensorinfoscreen.nodeinfo;
        }
        Row{
            spacing: 10;
            SUButton{
                text: qsTr("Toggle");
                width: 150;
                onClicked: {
                    activeSensor.toggleRelay();
                }
            }
            SUButton{
                text: qsTr("Set ON");
                width: 150;
                onClicked: {
                    activeSensor.setOn();
                }
            }
            SUButton{
                text: qsTr("Set OFF");
                width: 150;
                onClicked: {
                    activeSensor.setOff();
                }
            }
        }
    }
}

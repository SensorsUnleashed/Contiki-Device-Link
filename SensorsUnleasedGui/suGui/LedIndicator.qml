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
                text: qsTr("Toggle RED Led");
                width: 175;
                onClicked: {
                    activeSensor.toggleRedLED();
                }
            }
            SUButton{
                text: qsTr("Toggle Green Led");
                width: 175;
                onClicked: {
                    activeSensor.toggleGreenLED();
                }
            }
            SUButton{
                text: qsTr("Toggle Orange Led");
                width: 175;
                onClicked: {
                    activeSensor.toggleOrangeLED();
                }
            }
            SUButton{
                text: qsTr("Toggle Yellow Led");
                width: 175;
                onClicked: {
                    activeSensor.toggleYellowLED();
                }
            }
        }
    }
}

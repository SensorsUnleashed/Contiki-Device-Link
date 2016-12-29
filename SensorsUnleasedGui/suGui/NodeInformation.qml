import QtQuick 2.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2

Item{
    function addSensor(info){
        var sensorComponent = Qt.createComponent("Sensor.qml");
        if(sensorComponent.status === Component.Ready) {
            var sensor = sensorComponent.createObject(layout);
            sensor.texttop = info;
            sensor.identification = info;
            sensor.parentnode = nodeaddr;
            sensor.loader = sensorpopover;
            sensor.source = "SensorInformation.qml";
        }
    }

    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database

    Rectangle {
        id: nodeinfoscreen;
        color: suPalette.window;
        border.color: suPalette.buttonText;
        anchors.margins: 20;
        anchors.fill: parent;

        Column{
            width: 400;
            anchors.top: parent.top;
            anchors.left: parent.left;
            anchors.margins: 20;
            spacing: 15;
            Label{
                id: namefield;
                width: parent.width;
                font.pointSize: 14;
                text: nodeinfo["name"] + " ( " + nodeinfo["location"] + " )";
            }

            Label{
                id: addressfield;
                width: parent.width;
                font.pointSize: 10;
                font.italic: true;
                text: nodeinfo["address"];
            }

            Row{
                spacing: 30;

                SUButton{
                    text: qsTr("Refresh");
                    width: 100;
                    onClicked: {
                        activeNode.requestLinks();
                        layout.children = "";
                    }
                }

                SUButton{
                    text: qsTr("Back");
                    onClicked:{
                        activeNode.stopListening();
                        popover.source = "";
                    }
                }
            }

            Flow {
                id: layout;
                spacing: 10
            }

        }

        Component.onCompleted: {
            //Get the info from the node class in c++
            activeNode.getSensorslist();
        }

        Connections{
            target: activeNode;
            onSensorFound:{
                addSensor(sensorinfo);
            }
        }
    }

    Loader{
        id: sensorpopover;
        width: parent.width * 0.9;
        height: parent.height * 0.9;
        anchors.centerIn: parent;
        onStatusChanged: {
            if(sensorpopover.status == Loader.Ready)
                nodeinfoscreen.enabled = false;
            else if(sensorpopover.status == Loader.Null)
                nodeinfoscreen.enabled = true;
        }
    }
}

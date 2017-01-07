import QtQuick 2.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2

Item{
    height: sensorinfoscreen.height;
    width: parent.width;

    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database

    SUButton{
        id: backbutton;
        anchors.right: parent.right;
        text: qsTr("Back");
        onClicked: {
            activeSensor.stopListening();
            sensorpopover.source = "";
        }
    }

    CoapCommStatus{
        anchors.right: backbutton.left;
        deviceptr: activeSensor;
    }

    Column{
        id: sensorinfoscreen;

        spacing: 15;
        Label{
            id: namefield;
            anchors.left: parent.left;
            //width: parent.width;
            font.pointSize: 14;
            text: nodeaddr;
        }

        Row{
            spacing: 25;
            GroupBox{
                width: 150;
                height: 100;
                title: qsTr("Value");

                Label{
                    id: currentvallbl;
                    width: parent.width;
                    height: parent.height;
                    horizontalAlignment: Text.AlignHCenter;
                    verticalAlignment: Text.AlignVCenter;
                    text: "NA";
                    font.pointSize: 16;
                }
                MouseArea{
                    anchors.fill: parent;
                    onClicked: activeSensor.requestValue();
                }
                Connections{
                    target: activeSensor;
                    onCurrentValueChanged:{
                        currentvallbl.text = result["value"];
                    }
                }
            }
        }

        Row{
            spacing: 20;
            width: parent.width;
            SensorConfig{
                id: configwidget;
            }
            SensorPairing{
                id: pairingwidget;
                height: configwidget.height;
            }
        }

        Component{
            id: valuebox;
            GroupBox{
                width: 150;
                height: 100;
                title: qsTr("Value");
                Label{
                    width: parent.width;
                    height: parent.height;
                    horizontalAlignment: Text.AlignHCenter;
                    verticalAlignment: Text.AlignVCenter;
                    text: value;
                    font.pointSize: 16;

                }
            }
        }
    }
}



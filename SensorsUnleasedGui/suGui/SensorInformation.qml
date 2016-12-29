import QtQuick 2.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2

Rectangle {
    id: sensorinfoscreen;
    color: suPalette.window;
    anchors.margins: 20;

    border.color: suPalette.buttonText;

    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database

    Column{
        width: 600;
        anchors.top: parent.top;
        anchors.left: parent.left;
        anchors.margins: 20;
        spacing: 15;
        Label{
            id: namefield;
            width: parent.width;
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

        SensorConfig{
            id: configwidget;
            width: parent.width;
        }

        Row{
            spacing: 30;

//            SUButton{
//                text: qsTr("Refresh min/max");
//                width: 150;
//                onClicked: {
//                    activeSensor.requestRangeMin();
//                    activeSensor.requestRangeMax();
//                }
//            }

            SUButton{
                text: qsTr("Read sensor");
                width: 150;
                onClicked: {
                    activeSensor.req_eventSetup();
                }
            }
            SUButton{
                text: qsTr("Back");
                onClicked: {
                    activeSensor.stopListening();
                    sensorpopover.source = "";
                }
            }
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



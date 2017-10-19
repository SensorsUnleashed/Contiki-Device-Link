import QtQuick 2.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

Item{
    anchors.margins: 20;
    anchors.fill: parent;

    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database
    property var source_sensor_specific;

    function refresh(){
        console.log("Implement this");
    }

    function goBack(){
        activeSensor.stopListening();
        sensorpopover.source = "";
    }

    function endPair(){
        sensorinfolayout.currentIndex = 0;
    }

    StackLayout {
        id: sensorinfolayout;
        //columns: 2
        anchors.top: parent.top;
        anchors.fill: parent;
        Layout.fillWidth: true;
        Layout.fillHeight: true;
        Layout.preferredWidth: parent.width;
        Layout.preferredHeight: parent.height;


        ColumnLayout{   //Index 0
            id: sensorinfoscreen;
            Layout.fillWidth: false;
            Layout.preferredWidth: parent.width / 2;
            Layout.preferredHeight: parent.height;

            spacing: 15;

            Row{
                spacing: 25;
                GroupBox{
                    id: valuegroupbox;
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

                GroupBox{
                    width: 150;
                    height: 100;
                    title: qsTr("Pairings/Bindings");

                    SUButton{
                        text: "Setup";
                        anchors.centerIn: parent;
                        onClicked: sensorinfolayout.currentIndex = 1;
                    }
                }
            }

            Row{
                spacing: 20;
                width: parent.width;
                SensorConfig{
                    id: configwidget;
                }
            }

            GroupBox{
                title: qsTr("Test events");

                Row{
                    spacing: 10;
                    SUButton{
                        width: 140;
                        text: "Above Event"
                        onClicked: {
                            activeSensor.testEvents("aboveEvent", 0);
                        }
                    }
                    SUButton{
                        width: 140;
                        text: "Below Event"
                        onClicked: {
                            activeSensor.testEvents("belowEvent", 0);
                        }
                    }
                    SUButton{
                        text: "Change Event"
                        width: 140;
                        onClicked: {
                            activeSensor.testEvents("changeEvent", 0);
                        }
                    }
                }
            }

            GroupBox{
                title: "Test Actions"
                Loader{
                    source: source_sensor_specific;
                }
            }
            onVisibleChanged: if(visible){
                                  backbutton.command = goBack;
                                  refreshbutton.visible = true;
                                  refreshbutton.command = refresh;
                              }
        }

        SensorPairing{  //Index 1
            id: pairingwidget;
            Layout.preferredWidth: parent.width / 2;
            Layout.preferredHeight: parent.height;

            onVisibleChanged: if(visible) {
                                  backbutton.command = endPair
                              }
        }
    }

    Component{
        id: sensorheader_mid;
        Label{
            id: namefield;
            font.pointSize: 14;
            text: nodeaddr;
        }
    }

    Component{
        id: sensorheader_right;
        CoapCommStatus{
            deviceptr: activeSensor;
        }
    }

    Component.onCompleted: {
        header.headermid = sensorheader_mid;
        header.headerright = sensorheader_right;
    }
}



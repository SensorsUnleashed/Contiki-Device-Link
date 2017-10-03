import QtQuick 2.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

Rectangle{
    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database
    property var source_sensor_specific;

    color: suPalette.window;
    border.color: suPalette.buttonText;

    Component.onCompleted: {
        console.log("SensorInformation width: " + width + " height: " + height);
        console.log("SensorInformation width: " + width + " height: " + height);
    }
    onWidthChanged: console.log("SensorInformation changed width: " + width + " height: " + height);
    onHeightChanged: console.log("SensorInformation changed width: " + width + " height: " + height);


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

    GridLayout {
        id: grid
        columns: 2
        anchors.top: parent.top;
        anchors.fill: parent;
        Layout.fillWidth: true;
        Layout.fillHeight: true;
        Layout.preferredWidth: parent.width;
        Layout.preferredHeight: parent.height;

        onWidthChanged: console.log("GridLayout width: " + width + " height: " + height);
        onHeightChanged: console.log("GridLayout width: " + width + " height: " + height);

        ColumnLayout{
            id: sensorinfoscreen;
            Layout.fillWidth: false;
            Layout.preferredWidth: parent.width / 2;
            Layout.preferredHeight: parent.height;

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

                CheckBox {
                    id: observecheckbox;
                    property var observetoken;
                    width: 140;
                    checked: false
                    text: qsTr("Observe")
                    onCheckedChanged: {
                        if(checkState == Qt.Checked){
                            observetoken = activeSensor.requestObserve();
                            enabled = false;
                        }
                        else{
                            activeSensor.abortObserve(observetoken);
                        }
                    }
                    Connections{
                        target: activeSensor;
                        onObserve_started: {
                            observecheckbox.checked = true;
                            observecheckbox.enabled = true;
                        }
                        onObserve_failed: {
                            observecheckbox.checked = false;
                            observecheckbox.enabled = true;
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
                title: "Commands"
                Loader{
                    source: source_sensor_specific;
                }
            }
        }

        SensorPairing{
            id: pairingwidget;
            Layout.preferredWidth: parent.width / 2;
            Layout.preferredHeight: parent.height;
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



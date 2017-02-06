import QtQuick 2.7
import QtQuick.Controls 2.0

GroupBox{
    title: qsTr("Pairings:");
    width: 350;
    height: 350;

//    ListModel{
//        id: pairingslist;
//        ListElement{
//            nodename: "Test node";
//            sensorname: "sensor name";
//        }
//    }

    ListView{
        anchors.fill: parent;
        height: parent.height;
        width: parent.width;

        model: pairlist;//pairingslist;

        delegate: Column{
            Text {
                text: sensorname;
                font.pointSize: 12;
            }
            Text {
                text: nodename + " (" + nodeip + ")";
                font.pointSize: 8;
                font.italic: true;
            }
        }

        footerPositioning: ListView.OverlayFooter;
        footer: Row{
            width: parent.width;
            height: 50;
            spacing: 5;
            SUButton{
                text: "New pairing";
                width: parent.width / 2 - 10;
                onClicked: {
                    globalpopup.sourceComponent = pairingpicker;
                }
            }
            SUButton{
                text: "Get list";
                width: parent.width / 2 - 10;
                onClicked: {
                    activeSensor.getpairingslist();
                }
            }
        }
    }

//    Connections{
//        target: activeSensor;
//        onPairlistUpdated:{
//            pairingslist.clear();

//            //var plist = activeSensor.getPairList();
//            //console.log(plist);
//        }
//    }

    Component{
        id: pairingpicker

        MouseArea{
            Rectangle{
                width: 500;
                height: 500;
                anchors.centerIn: parent;
                color: suPalette.window;
                border.color: suPalette.buttonText;

                SensorList{
                    anchors.fill: parent;
                }
            }
        }
    }
}

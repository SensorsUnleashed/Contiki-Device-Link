import QtQuick 2.7
import QtQuick.Controls 2.0

GroupBox{
    title: qsTr("Pairings:");
    width: 350;
    height: 350;

    ListView{
        id: lw;
        anchors.fill: parent;
        height: parent.height;
        width: parent.width;

        model: pairlist;

        delegate:Rectangle{
            width: parent.width;
            height: item.height;
            color: selected == 0 ? "transparent" : "grey";
            Column{
                id: item;
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
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    pairlist.setSelected(index, !selected); //Toggle
                }
            }
        }

        footerPositioning: ListView.OverlayFooter;
        footer: Row{
            width: parent.width;
            height: 50;
            spacing: 5;
            SUButton{
                text: "New";
                width: parent.width / 3 - 10;
                onClicked: {
                    globalpopup.sourceComponent = pairingpicker;
                }
            }
            SUButton{
                text: "Refresh";
                width: parent.width / 3 - 10;
                onClicked: {
                    activeSensor.getpairingslist();
                }
            }
            SUButton{
                text: "Remove selected";
                width: parent.width / 3 - 10;
                onClicked: {
                    //activeSensor.clearpairingslist();
                    pairlist.removePairings();
                }
            }
//            SUButton{
//                text: "Test";
//                width: parent.width / 3 - 10;
//                onClicked: {
//                    activeSensor.removeItems();
//                }
//            }
        }
    }

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

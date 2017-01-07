import QtQuick 2.7
import QtQuick.Controls 2.0

GroupBox{
    title: qsTr("Pairings:");
    width: 350;
    height: 350;

    ListModel{
        id: pairingslist;
        ListElement{
            nodename: "Test node";
            sensorname: "sensor name";
        }
    }

    ListView{
        anchors.fill: parent;
        height: parent.height;
        width: parent.width;

        model: pairingslist;

        delegate: Column{
            Text {
                text: sensorname;
                font.pointSize: 12;
            }
            Text {
                text: nodename;
                font.pointSize: 8;
                font.italic: true;
            }
        }

        footerPositioning: ListView.OverlayFooter;
        footer:  SUButton{
            text: "New pairing";
            width: parent.width;
            onClicked: {
                globalpopup.sourceComponent = pairingpicker;
            }
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

import QtQuick 2.3
import QtQuick.Controls 2.0

Rectangle {
    id: newnodescreen;
    color: suPalette.window;
    border.width: 1;
    anchors.margins: 20;

    property string nodeaddr: "";

    Column{
        width: 400;
        anchors.verticalCenter: parent.verticalCenter;
        anchors.horizontalCenter: parent.horizontalCenter;
        spacing: 15;
        TextField{
            id: namefield;
            width: parent.width;
            font.pointSize: 14;
            placeholderText: qsTr("Name of the node");
        }

        TextField{
            id: locationfield;
            width: parent.width;
            font.pointSize: 14;
            placeholderText: qsTr("Node location");
        }

        TextField{
            id: addressfield;
            width: parent.width;
            font.pointSize: 14;
            placeholderText: qsTr("IPv6 address of node");
        }

        Row{
            spacing: 30;

            SUButton{
                text: qsTr("Create");
                onClicked: {
                    onClicked: newnodescreen.visible = false;
                    var nodeinfo = {
                        name: namefield.text,
                        location: locationfield.text,
                        address: addressfield.text
                    }
                    su.createNode(nodeinfo);
                }
            }

            SUButton{
                text: qsTr("Cancel");
                onClicked: {
                    popover.source = "";
                }
            }
        }
    }
}




import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQml.Models 2.2
import QtQuick.Layouts 1.3

ListView{
    id: lw;
    anchors.fill: parent;

    signal addNew;
    signal select(var data);

    model: pairlist;

    delegate:Rectangle{
        width: parent.width;
        height: item.height;
        //color: selected == 0 ? "transparent" : "grey";
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
                var pairdata = {};
                pairdata['addr'] = nodeip;
                pairdata['url'] = sensorname;
                select(pairdata);
            }
        }
    }

    footerPositioning: ListView.OverlayFooter;
    footer: RowLayout{
        Layout.fillWidth: true;
        Layout.preferredWidth: lw.width;
        Layout.preferredHeight: 50;
        spacing: 5;
        SUButton{
            text: "ADD";
            Layout.fillWidth: true;
            Layout.preferredWidth: lw.width;
            onClicked: {
                addNew();
            }
        }

        //                SUButton{
        //                    text: "Refresh";
        //                    Layout.fillHeight: true;
        //                    Layout.preferredWidth: parent.width / 2;
        //                    onClicked: {
        //                        activeSensor.getpairingslist();
        //                    }
        //                }
    }
}

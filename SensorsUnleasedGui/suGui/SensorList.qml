import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQml.Models 2.2

Item{
    id: sensorlistview;

    signal cancel;
    signal select(var data);

    ListModel{
        id: sensorlistmodel;
        Component.onCompleted: {
            var list = su.getAllSensorsList();
            for (var i=0; i<list.length; i++){
                append({
                           "node_name": list[i]["node_name"],
                           "node_addr": list[i]["node_addr"],
                           "sensor_name": list[i]["sensor_name"],
                       })
            }
        }
    }

    ListView{
        id: lw;
        anchors.fill: parent;
        anchors.margins: 5;
        height: parent.height;
        width: parent.width;

        model: sensorlistmodel;

        highlight: Rectangle {
            color: "lightgrey";
        }
        highlightFollowsCurrentItem: true;
        focus: true

        delegate: Item{
            width: parent.width;
            height: item.height;
            Column{
                id: item;
                Text {
                    text: sensor_name;
                    font.pointSize: 12;
                }
                Text {
                    text: node_name + " (" + node_addr + ")";
                    font.pointSize: 8;
                    font.italic: true;
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lw.currentIndex = index;
                }
            }
        }
        footerPositioning: ListView.OverlayFooter;
        footer: Row{
            width: sensorlistview.width - 10;
            spacing: 10;
            SUButton{
                text: "NEXT";
                width: (parent.width - parent.spacing) / 2 -1;
                onClicked: {
                    //For now we just start the pairing process
                    var pairdata = {};
                    pairdata['addr'] = sensorlistmodel.get(lw.currentIndex).node_addr;
                    pairdata['url'] = sensorlistmodel.get(lw.currentIndex).sensor_name;
//                    activeSensor.pair(pairdata);
                    select(pairdata)
                }
            }
            SUButton{
                text: "CANCEL"
                width: (parent.width - parent.spacing) / 2 -1;
                onClicked: {
                    cancel();
                }
            }
        }
    }
}

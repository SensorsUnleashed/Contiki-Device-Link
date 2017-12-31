import QtQuick 2.0

Item {
    GridView {
        anchors.fill: parent;
        model: nodesmodel;
        cellHeight: 170;
        cellWidth: 170;
        delegate: Column {
            Node{
                texttop: Name;
                textmid: Location;
                textbot: Qt.formatDateTime(LastSeen, "dd-MM HH:mm");
                identification: IPAdress;
                loader: popover;
                source: "NodeInformation.qml";
                nodeinfo: NodeInfo;
                statbar: BorderColor;
            }
        }
    }
}

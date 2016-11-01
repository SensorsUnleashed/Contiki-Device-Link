import QtQuick 2.0

MouseArea{
    width: 1024;
    height: 800;

    property alias contentsource : overlaycontentloader.source;

    Rectangle{
        anchors.fill: parent;
        opacity: 0.5;
        color: "grey";
    }

    Rectangle{
        anchors.centerIn: parent;
        color: "white";
        border.color: "black";
        border.width: 2;

        width: parent.width * 0.7;
        height: parent.height * 0.7;

        Loader{
            id: overlaycontentloader;
            width: parent.width;
            height: parent.height;
            source: "";
        }
    }
}

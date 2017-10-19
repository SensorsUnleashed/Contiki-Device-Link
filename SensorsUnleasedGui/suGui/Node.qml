import QtQuick 2.7
import QtQuick.Controls 2.0


Button {
    width: 150;
    height: 150;

    property alias texttop: top.text;
    property alias textmid: mid.text;
    property alias textbot: bot.text;

    property var identification;

    property var loader;
    property var source;

    property var nodeinfo;  //As received from the database

    Column{
        anchors.centerIn: parent;
        spacing: 5;
        Label{
            id: top;
            text: "";
            color: suPalette.buttonText;
            font.pointSize: 12;
        }
        Label{
            id: mid;
            text: "";
            color: suPalette.buttonText;
            font.pointSize: 12;
        }
        Label{
            id: bot;
            text: "";
            color: suPalette.buttonText;
            font.pointSize: 12;
        }
    }

    onClicked: {
        su.changeActiveNode(nodeinfo);
        loader.setSource(source, {nodeaddr: identification, nodeinfo: nodeinfo});
    }
}

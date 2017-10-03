import QtQuick 2.7
import QtQuick.Controls 2.0

Button {
    width: 150;
    height: 150;

    property alias texttop: top.text;
    property alias textmid: mid.text;
    property alias textbot: bot.text;

    property var identification;
    property var parentnode;

    property var loader;
    property var source_common;
    property var source_sensor_specific;

    property var nodeinfo;  //As received from the database

    Column{
        anchors.centerIn: parent;
        spacing: 5;
        Label{
            id: top;
            text: "";
            font.pointSize: 12;
        }
        Label{
            id: mid;
            text: "";
            font.pointSize: 12;
        }
        Label{
            id: bot;
            text: "";
            font.pointSize: 12;
        }
    }

    onClicked: {
        var sensorinfo = {
            node: parentnode,
            sensor: identification,
        }
        su.changeActiveSensor(sensorinfo);
        loader.setSource(source_common, {nodeaddr: identification, nodeinfo: nodeinfo, source_sensor_specific: source_sensor_specific});
    }
}

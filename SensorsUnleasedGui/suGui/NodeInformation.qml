import QtQuick 2.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

StackLayout{
    id: nodeinfoscreen;
    anchors.fill: parent;

    function back(){
        activeNode.stopListening();
        popover.source = "";
    }

    function refresh(){
        activeNode.requestLinks();
        layout.children = "";
    }

    function addSensor(info, source){
        var sensorComponent = Qt.createComponent("Sensor.qml");
        if(sensorComponent.status === Component.Ready) {
            var sensor = sensorComponent.createObject(layout);
            sensor.texttop = info;
            sensor.identification = info;
            sensor.parentnode = nodeaddr;
            sensor.loader = sensorpopover;
            sensor.source_common = "SensorInformation.qml";
            sensor.source_sensor_specific = source;
        }
    }

    property string nodeaddr: "";
    property var nodeinfo;  //As received from the database

    ColumnLayout  { //Index 0
        anchors.fill: parent;
        spacing: 15;
        Flow {
            id: layout;
            spacing: 10
            Layout.fillHeight: true;
            Layout.fillWidth: true;
        }

        Connections{
            target: activeNode;
            onSensorFound:{
                addSensor(sensorinfo, source);
            }
        }
        onVisibleChanged: {
            if(visible){
                header.headerleft = nodesheaderleft;
                header.headerright = nodesheaderright;
                header.headermid = frontheader_mid;
                backbutton.command = back;
                configbut.visible = true;
            }
        }
    }

    Loader{ //Index 1
        id: sensorpopover;
        anchors.fill: parent;
        onStatusChanged: {
            if(sensorpopover.status == Loader.Ready)
                parent.currentIndex = 1;
            else if(sensorpopover.status == Loader.Null){
                parent.currentIndex = 0;
                backbutton.command = back;
                refreshbutton.command = refresh;
            }
        }
    }

    Loader{
        id: nodeconfig;
        property var previndex;
        anchors.fill: parent;

        onStatusChanged: {
            if(status == Loader.Ready){
                previndex = nodeinfoscreen.currentIndex;
                nodeinfoscreen.currentIndex = 2;
            }
            else if(status == Loader.Null)
                nodeinfoscreen.currentIndex = previndex;
        }
    }

    Component.onCompleted: {
        //Get the info from the node class in c++
        activeNode.getSensorslist();
        backbutton.command = back;
        refreshbutton.command = refresh;
    }

//    SUButton{
//        id: refreshbutton;
//        parent: bottombar
//        property var command: null;
//        visible: true;
//        text: qsTr("Refresh");
//        onClicked:{
//            if(command !== null)
//                command();
//        }
//    }

    SUButton{
        id: configbut;
        parent: settingsplaceholder;
        text: qsTr("Config");
        onClicked: {
            nodeconfig.setSource("NodeConfig.qml");
        }
    }

    Component{
        id: nodesheaderleft;
        ColumnLayout  {
            spacing: 5;

            Label{
                id: namefield;
                font.pointSize: 14;
                text: nodeinfo["name"] + " ( " + nodeinfo["location"] + " )";
            }

            Label{
                id: addressfield;
                font.pointSize: 10;
                font.italic: true;
                text: nodeinfo["address"];
            }
        }
    }

    Component{
        id: nodesheaderright

        CoapCommStatus{
            deviceptr: activeNode;
        }
    }
}

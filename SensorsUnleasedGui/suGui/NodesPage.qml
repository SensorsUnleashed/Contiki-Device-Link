import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Item{
    function addNode(info){
        var nodeComponent = Qt.createComponent("Node.qml");
        if(nodeComponent.status === Component.Ready) {
            var node = nodeComponent.createObject(layout);
            for(var i in info){
                node.nodeinfo = info;
                if(i === "name")
                    node.texttop = info[i];
                else if(i === "location")
                    node.textmid = info[i];
                else if(i === "address")
                    node.identification = info[i];

                node.loader = popover;
                node.source = "NodeInformation.qml";
            }
        }
    }

    function refreshNodeslist(){
        console.log("refreshNodeslist")
        borderrouter.getNodeslist();
    }


    StackLayout {
        id: nodepage;
        anchors.fill: parent;

        Flow {
            id: layout;
            anchors.fill: parent
            anchors.margins: 4
            spacing: 10

            Node{
                texttop: "New Node";
                loader: popover;
                source: "NewNodeScreen.qml";
            }

            onVisibleChanged: {
                if(visible) refreshbutton.command = refreshNodeslist;
            }
        }

        Loader{
            id: popover;
            anchors.fill: parent;

            onStatusChanged: {
                if(popover.status == Loader.Ready){
                    nodepage.currentIndex = 1;
                    backbutton.visible = true;
                }
                else if(popover.status == Loader.Null){
                    nodepage.currentIndex = 0;
                    backbutton.visible = false;
                }
            }
        }
    }

    //    DropShadow {
    //        anchors.fill: nodepage
    //        horizontalOffset: 3
    //        verticalOffset: 3
    //        radius: 8.0
    //        samples: 17
    //        color: suPalette.shadow;
    //        source: nodepage;
    //    }

    Connections {
        target: su
        onNodeCreated:{
            addNode(nodeinfo);
        }
    }

    Component.onCompleted: {
        su.initNodelist();
        refreshbutton.command = refreshNodeslist;
    }
}



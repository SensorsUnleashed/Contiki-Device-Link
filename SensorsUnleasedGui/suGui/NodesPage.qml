import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Item{
    function refreshNodeslist(){
        console.log("refreshNodeslist")
        borderrouter.getNodeslist();
    }

    StackLayout {
        id: nodepage;
        anchors.fill: parent;

        NodeList{ //Index 0
            anchors.fill: parent;

            onVisibleChanged: {
                if(visible) refreshbutton.command = refreshNodeslist;
            }
        }

        Loader{ //Index 1
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

    Component.onCompleted: {
        refreshbutton.command = refreshNodeslist;
    }
}



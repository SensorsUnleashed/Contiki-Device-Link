import QtQuick.Controls 2.0
import QtQuick 2.7

Rectangle {
    width: 200;
    height: 300;
    color: "transparent";
    property string activeurl;
    property string activeip: ipaddr.activeNodeAddr;

    function updatedir(directorymap){
        for (var resource in directorymap) {
            var attributes = "";

            //console.log(resource);
            for (var subitem in directorymap[resource]){
                //console.log(directorymap[resource][subitem]);
                attributes += directorymap[resource][subitem] + "; "
            }
            contactmodel.append({"url":resource, "attributes":attributes});
        }
    }

    ListModel {
        id: contactmodel;
        //Populated in updatedir()
    }

    Column{
        anchors.fill: parent;
        spacing: 10;
        Ip6addrwidget{
            id: ipaddr;
            width: parent.width;
        }
        Rectangle{
            width: parent.width;
            height: parent.height - ipaddr.height - parent.spacing;
            ListView{
                id: list;
                spacing: 5;
                anchors.fill: parent;
                model: contactmodel;
                highlight: Rectangle { color: "lightsteelblue"; radius: 5}
                highlightFollowsCurrentItem: true;
                focus: true

                delegate: Text {
                    text: url;// + " " + attributes;
                    font.pointSize: 14;
                    height: 30;
                    width: parent.width;
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            list.currentIndex = index;
                            activeurl = contactmodel.get(index).url;
                        }
                    }
                }
                Component.onCompleted: list.currentIndex = -1;
            }
        }
        Connections {
            target: coap
            onCoapMessageRdy:{
                if(messageid === ipaddr.acceptid){
                    contactmodel.clear();
                    updatedir(coap.getNodeLinks(ipaddr.activeNodeAddr));
                    ipaddr.acceptid = -1;   //Rdy for next time
                }
            }
        }
    }
}

import QtQuick 2.7
import QtQuick.Controls 2.0


Item {

    Item{
        width: parent.width -40;
        height: parent.height -40;
        anchors.centerIn: parent;
        Row{
            id: row;
            width: parent.width;
            height: parent.height* 0.7;
            spacing: 20;

            ResourceDir{
                id: srcnode;
                width: parent.width * 0.425;
                height: parent.height;
            }
            Button{
                height: 100;
                width: parent.width * 0.15  - parent.spacing * 2;
                text: "PAIR";

                onClicked: {
                    dstnode.activeip = "fe80::2677:3ff:fe6c:20f0";
                    dstnode.activeurl = "relay/light"
                    srcnode.activeip = "fe80::2677:3ff:fe6c:20f0";
                    srcnode.activeurl = "button/actuator"

                    if(!dstnode.activeip || !dstnode.activeurl) return;
                    if(!srcnode.activeip || !srcnode.activeurl) return;

                    var pairdata = {};
                    pairdata['addr'] = dstnode.activeip;
                    pairdata['url'] = dstnode.activeurl;

                    var options = {};
                    options['ct'] = 42;  //COAP_CONTENT_FORMAT_APP_OCTET
                    options['type'] = 0; //COAP_CONFIRMABLE
                    options['code'] = 3; //COAP_PUT
                    var acceptid = su.pair(srcnode.activeip, srcnode.activeurl, options, pairdata);
                    //activeNodeAddr = ipaddr.text;
                }
            }
            ResourceDir{
                id: dstnode;
                width: parent.width * 0.425;
                height: parent.height;
            }
        }

        Button{
            anchors.top: row.bottom;
            text: "Coap Setup"
            onClicked: {
                //Set the overlaybox
                setupboxloader.source = "Overlaybox.qml";
                //Next make the overlaybox load the right contents.
                setupboxloader.item.contentsource = "CoapSetupContent.qml";
            }
        }
    }
}

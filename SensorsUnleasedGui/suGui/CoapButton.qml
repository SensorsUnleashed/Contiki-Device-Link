import QtQuick 2.0
import QtQuick.Controls 2.0

Button{
    width: 100;
    height: 50;

    property int acceptid: -1;
    property string requrl;
    property string ipaddr;
    property var options;
    onClicked: {
        if(nodesdir.activeurl){
            log.text += "Get " + nodesdir.activeurl + "\n";
            acceptid = coap.reqGet(ipaddr, requrl, options, acceptid);
        }
    }
    Connections {
        target: coap
        onCoapMessageRdy:{
            if(messageid === acceptid){
                log.text += coap.getNodeMessage(nodesdir.activeip) + '\n';
                reqbutton.acceptid = -1;   //Rdy for next time
            }
        }
    }
}

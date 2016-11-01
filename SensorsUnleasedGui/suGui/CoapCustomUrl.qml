import QtQuick 2.0
import QtQuick.Controls 2.0

Item{
    property int acceptid: -1;
    property string requrl;
    property string ipaddr;
    property var options;

    height: parent.height;
    width: parent.width;

    Rectangle {
        id:customurlbox;
        anchors.left: parent.left;
        width: parent.width * 0.8;
        height: 50;

        TextField {
            id:customurl;
            anchors.fill: parent;
            anchors.margins: 1
            placeholderText: qsTr("Enter custom uri");
            text: requrl;
        }
    }
    CoapButton{
        width: parent.width * 0.2 - 20;
        height: 50;
        anchors.right: parent.right;
        text: "Send";
        requrl: customurl.text;
        ipaddr: parent.ipaddr;
        options: {
            'ct': 0,    //COAP_CONTENT_FORMAT_TEXT_PLAIN
            'type': 0,  //COAP_CONFIRMABLE
            'code': 1,  //COAP_GET
        }
    }
}

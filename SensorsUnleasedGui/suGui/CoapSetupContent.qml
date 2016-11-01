import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    anchors.fill: parent;
    anchors.margins: 20;

    //16,32,64,128,256,512,1024
    Column{
        width: parent.width * 0.5;
        height: parent.height;

        GroupBox{
            width: parent.width;
            height: 150;
            title: qsTr("CoAP settings");
            Column{
                width: parent.width;
                //height: parent.height;
                SettingSlider{
                    id: coapPrefMsgSize;
                    name: qsTr("Pref. message size");
                    to: 10;
                    from: 4;
                    value: 5;
                    stepSize: 1;
                    resultval: Math.pow(2, value);
                }
                SettingSlider{
                    id: coapAckTimeout;
                    name: qsTr("ACK timeout");
                    to: 10;
                    from: 1;
                    value: 5;
                    stepSize: 1;
                }
            }
            Component.onCompleted: {
                var options = coap.getSettings();
                coapPrefMsgSize.value = options['coapPrefMsgSize'];
                coapAckTimeout.value = options['coapAckTimeout'];
            }
        }
    }

    Row{
        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        spacing: 10;

        Button{
            text: "Save";
            width: 100;
            height: 50;
            onClicked: {
                var settings = {
                    'coapPrefMsgSize' : parseInt(coapPrefMsgSize.resultval),
                    'coapAckTimeout' : parseInt(coapAckTimeout.resultval),
                }
                coap.updateSettings(settings);
                setupboxloader.source = "";
            }
        }
        Button{
            text: "Cancel";
            width: 100;
            height: 50;
            onClicked: {
                setupboxloader.source = "";
            }
        }
    }
}

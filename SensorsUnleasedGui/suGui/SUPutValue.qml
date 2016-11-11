import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    property int acceptid: -1;
    property string requrl;
    property string ipaddr;
    property var options;

    height: parent.height;
    width: parent.width;

    Row{
        height: parent.height;
        width: parent.width;
        spacing: 5;
        Rectangle {
            id: inputvalcont;
            width: parent.width * 0.4;
            height: parent.height;

            TextField {
                id:inputvalue;
                anchors.fill: parent;
                placeholderText: qsTr("Enter value");
                text: "";
            }
        }

        ComboBox {
            id: msgpackformat;
            width: parent.width * 0.4;
            height: parent.height;
            model: ListModel {
                id: formatitems;
                ListElement { type: "uin8_t"; typeindex: 14 }
                ListElement { type: "uint16_t"; typeindex: 15 }
            }

            delegate: ItemDelegate {
                width: msgpackformat.width
                text: type;
                font.weight: msgpackformat.currentIndex === index ? Font.DemiBold : Font.Normal
                highlighted: msgpackformat.highlightedIndex == index
            }

            textRole: "type"

        }

        Button{
            width: parent.width * 0.2;
            height: parent.height;
            text: "Send";

            onClicked: {
                //console.log(formatitems.get(msgpackformat.currentIndex).index);
                if(!ipaddr || !requrl || !inputvalue.text) return;

                var options = {
                    'ct': 42,       //COAP_CONTENT_FORMAT_APP_OCTET
                    'type': 0,      //0x10; //COAP_NON_CONFIRMABLE
                    'code': 3,      //COAP_PUT
                };

                var data = {
                    'value': Number(inputvalue.text),
                    'format':  formatitems.get(msgpackformat.currentIndex).typeindex,
                }

                acceptid = su.put(ipaddr, requrl, options, data, acceptid);
            }
        }
    }
}

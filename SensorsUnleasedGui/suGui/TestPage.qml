import QtQuick 2.7
import QtQuick.Controls 2.0


Item {
    Item{
        width: parent.width -40;
        height: parent.height -40;
        anchors.centerIn: parent;
        Row{
            id: row1;
            width: parent.width;
            height: parent.height* 0.7;
            spacing: 20;

            ResourceDir{
                id: nodesdir;
                width: parent.width * 0.425;
                height: parent.height;
            }

            DebugOutput{
                id: log;
                width: parent.width * 0.575 - parent.spacing;
                height: parent.height;
            }
        }
        Row{
            id: row2;
            width: parent.width;
            anchors.topMargin: 20;
            height: 50;
            anchors.top: row1.bottom;
            spacing: 5;

            ComboBox {
                id: content;
                width: 150;
                height: parent.height;
                model: ListModel {
                    id: formatitems;
                    ListElement { format: "Text Plain"; formatindex: 0 }
                    ListElement { format: "MsgPack"; formatindex: 42 }
                }
                delegate: ItemDelegate {
                    width: content.width
                    text: format
                    font.weight: content.currentIndex === index ? Font.DemiBold : Font.Normal
                    highlighted: content.highlightedIndex == index
                }

                textRole: "format"
            }

            ComboBox {
                id: query;
                width: 150;
                height: parent.height;
                model: ListModel {
                    id: queryitems;
                    ListElement { q_command: "Empty"; }
                    ListElement { q_command: "AboveEventAt"; }
                    ListElement { q_command: "BelowEventAt"; }
                    ListElement { q_command: "ChangeEventAt"; }
                }
                delegate: ItemDelegate {
                    width: query.width
                    text: q_command
                    font.weight: query.currentIndex === index ? Font.DemiBold : Font.Normal
                    highlighted: query.highlightedIndex == index
                }

                textRole: "q_command"
            }

            CoapButton{
                id: reqbutton;
                text: "Get";
                requrl: queryitems.get(query.currentIndex).q_command === "Empty" ? nodesdir.activeurl : nodesdir.activeurl + "?" + queryitems.get(query.currentIndex).q_command;
                ipaddr: nodesdir.activeip;
                options: {
                    'ct': formatitems.get(content.currentIndex).formatindex,
                    'type': 0,  //COAP_CONFIRMABLE
                    'code': 1,  //COAP_GET
                }
            }
            CoapButton{
                text: "Put large";
                requrl: nodesdir.activeurl;
                ipaddr: nodesdir.activeip;
                options: {
                    'ct': 0,    //COAP_CONTENT_FORMAT_TEXT_PLAIN
                    'type': 0,  //COAP_CONFIRMABLE
                    'code': 3,  //COAP_PUT
                }
                payload: "Dette er en test for at se om vi kan sende en lang tekst";
            }


        }
        Column{
            id: row3;
            width: parent.width;
            anchors.topMargin: 20;
            height: 100;
            anchors.top: row2.bottom;
            spacing: 5;

            CoapCustomUrl{
                height: 50;
                requrl: nodesdir.activeurl;
                ipaddr: nodesdir.activeip;
                options: {
                    'ct': 0,    //COAP_CONTENT_FORMAT_TEXT_PLAIN
                    'type': 0,  //COAP_CONFIRMABLE
                    'code': 1,  //COAP_GET
                }
            }
            SUPutValue{
                height: 50;
                width: 400;
                requrl: nodesdir.activeurl;
                ipaddr: nodesdir.activeip;
            }

            Row{
                width: parent.width;
                height: 30;
                spacing: 5;
                Label{
                    id: retranscount;
                    height: parent.height;
                    width: 100;
                    text: "Retranscount";

                    Connections {
                        target: coap
                        onCoapRetrans: {
                            retranscount.text = count + "(" + outof + ")";
                        }
                    }
                }
                Label{
                    id: coapcode;
                    height: parent.height;
                    width: 100;
                    text: "Code";
                    Connections {
                        target: coap
                        onCoapCode: {
                            coapcode.text = code;
                        }
                    }
                }
                Label{
                    height: parent.height;
                    width: 100;
                    text: "Processcount";
                }

            }


        }
    }
}

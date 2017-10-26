import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ColumnLayout {
    function back(){
        backbutton.command = prevback;
        configbut.visible = true;
        nodeconfig.source = "";
    }

    function parseRequest(req, data){

        if(req === "req_versions"){
            boardname.text = data[0]["value"];
            swver.text = data[1]["value"][0] + ".";
            swver.text += data[1]["value"][1] + ".";
            swver.text += data[1]["value"][2];
        }
        else if(req === "req_coapstatus"){
            obscount.text = data[0]["value"][0];
            observeescount.text = data[0]["value"][1];
            pairtotcount.text = data[0]["value"][2];
        }
    }

    property var prevback;

    GroupBox{
        title: "Tools";
        RowLayout{
            spacing: 10;
            SUButton{
                text: qsTr("Flash format");
                onClicked:{
                    configdev.request_cfs_format();
                }
            }

            SUButton {
                text: "Observe Retry"
                onClicked: {
                    configdev.request_observe_retry();
                }
            }

            SUButton{
                text: "SW Update";
                enabled: false;
            }

            SUButton{
                text: "SW Reset";
                enabled: false;
            }
        }
    }

    GroupBox{
        title: "Versions";

        Grid {
            columns: 2;
            spacing: 25;

            Label{ text: "Board:" }
            Label{ text: " "; id: boardname; }

            Label{ text: "SW:" }
            Label{ text: " "; id: swver; }

            SUButton{
                text: "Refresh";
                onClicked:
                configdev.request_versions();
            }
        }
    }

    GroupBox{
        title: "Coap Status";

        Grid {
            columns: 2;
            spacing: 25;

            Label{ text: "Observers:" }
            Label{ text: "0"; id: obscount; }

            Label{ text: "Observees:" }
            Label{ text: "0"; id: observeescount; }

            Label{ text: "Total pairs count:"; }
            Label{ text: "0"; id: pairtotcount; }

            SUButton{
                text: "Refresh";
                onClicked:
                configdev.request_coapstatus();
            }
        }


    }

    GroupBox{
        title: "RPL Status";
    }

    Connections{
        target: configdev;
        onRequst_received: parseRequest(req, result);
    }

    Component.onCompleted: {
        prevback = backbutton.command;
        backbutton.command = back;
        configbut.visible = false;
    }
}

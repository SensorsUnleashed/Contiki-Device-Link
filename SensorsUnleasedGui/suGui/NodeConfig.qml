import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ColumnLayout {
    function back(){
        parent.currentIndex = 0;
    }

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
            }
        }
    }

    GroupBox{
        title: "Versions";
        SUButton{
            text: "Refresh";
            onClicked:
            configdev.request_versions();
        }
    }

    GroupBox{
        title: "Coap Status";
        SUButton{
            text: "Refresh";
            onClicked:
            configdev.request_coapstatus();
        }
    }

    GroupBox{
        title: "RPL Status";
    }


    onVisibleChanged: {
        if(visible){
            backbutton.command = back;
            configbut.visible = false;
        }
    }
}

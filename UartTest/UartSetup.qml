import QtQuick 2.0
import QtQuick.Controls 1.2

Item{
    Column{
        width: parent.width;
        height: parent.width;
        spacing: 10;

        Row{
            width: parent.width;
            height: 30;
            Button{
                width: parent.width / 3;
                height: parent.height;
                text: "Toggle DTR";
                onClicked:uart.setup("DTR Toggle");
            }
            Button{
                width: parent.width / 3;
                height: parent.height;
                text: "Toggle RTS";
                onClicked:uart.setup("RTS Toggle");
            }
        }
        Rectangle{
            id: commstatus;
            width: 30;
            height: 30;
            color: "red";
            border.color: "#64892E";
            border.width: 2;

            Timer{
                id: commalivetimer;
                interval: 1000;    //12,00004 hours (725 px wide plot)
                running: false;
                repeat: false;
                onTriggered: {
                    commstatus.color = "red";
                }
            }

            Connections {
                target: uart
                onMessageReceived:{
                    commstatus.color = "green";
                    commalivetimer.restart();
                }
            }
        }
    }
}


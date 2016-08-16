import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2

ApplicationWindow {
    title: qsTr("Power Monitor")
    width: 640
    height: 480
    visible: true
    color: "#497607";

    //75B616	64892E	497607	A0DB4A	AFDB71

    menuBar: MenuBar {

        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("Hjem");
                onTriggered: mainScreen.state = "overview";
            }

            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("&Communication")
            MenuItem {
                text: qsTr("&Setup uart")
                onTriggered: mainScreen.state = "uartsetup";
            }
            MenuItem {
                text: qsTr("&Test protocol")
                onTriggered: mainScreen.state = "dlt654test";
            }
        }
    }


    Item{
        id: mainScreen;
        width: parent.width - 20;
        height: parent.height -20;
        anchors.centerIn: parent;

        Loader{
            id: screenloader;
            source: "";
            width: parent.width;
            height: parent.height;
        }

        state: "overview"
        states: [
            State {
                name: "overview"
                PropertyChanges {target: screenloader; source: "SensActTest.qml"; }
            },
            State {
                name: "uartsetup"
                PropertyChanges {target: screenloader; source: "UartSetup.qml"; }
            },
            State {
                name: "dlt654test"
                PropertyChanges {target: screenloader; source: "DLT645Tester.qml"; }
            }

        ]
    }
}

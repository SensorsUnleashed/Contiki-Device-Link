import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: mainwindow;
    //Material.theme: Material.dark;
    SystemPalette {
        id: suPalette;
        colorGroup: SystemPalette.Active
    }

    visible: true
    width: 768; //;Screen.width;
    height: 768; //Screen.height;
    color: suPalette.window;
    title: qsTr("Sensors Unleased Setup Tool")

    property Component prevBottombar;

    function resetHeader(){
        header.headerleft = frontheader_left;
        header.headermid = frontheader_mid;
        header.headerright = frontheader_right;
    }

    Item{
        id: header;
        anchors.top: parent.top;
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.margins: 10;
        height: 50;

        property Component headerleft: frontheader_left;
        property Component headermid: frontheader_mid;
        property Component headerright: frontheader_right;

        Loader{
            sourceComponent: header.headerleft;
            anchors.left: parent.left;
            anchors.verticalCenter: parent.verticalCenter;
        }
        Loader{
            sourceComponent: header.headermid;
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.verticalCenter: parent.verticalCenter;
        }
        Loader{
            sourceComponent: header.headerright;
            anchors.right: parent.right;
            anchors.verticalCenter: parent.verticalCenter;
        }
    }

    NodesPage{
        anchors.top: header.bottom;
        anchors.bottom: bottombar.top;
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.margins: 10;
    }

    Item{
        id: settingsplaceholder;
        anchors.bottom: parent.bottom;
        anchors.left: parent.left;
        height: bottombar.height;
        anchors.margins: 10;
        width: 100;
    }

    Flow{
        id:  bottombar;

        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        anchors.left: settingsplaceholder.right;
        height: 50;
        anchors.margins: 10;
        layoutDirection: Qt.RightToLeft;
        spacing: 20;

        SUButton{
            id: backbutton;
            property var command: null;
            visible: false;
            text: qsTr("Back");
            onClicked:{
                if(command !== null)
                    command();
            }
        }

        SUButton{
            id: refreshbutton;
            parent: bottombar
            property var command: null;
            visible: true;
            text: qsTr("Refresh");
            onClicked:{
                if(command !== null)
                    command();
            }
        }
    }


    Component{
        id: frontheader_left;
        Label{
            text: ""
            font.pointSize: 18;
        }
    }

    Component{
        id: frontheader_mid;
        Label{
            text: "Sensors Unleashed"
            font.pointSize: 18;
            font.italic: true;
            font.underline: true;
        }
    }

    Component{
        id: frontheader_right;
        Label{
            text: ""
            font.pointSize: 18;
        }
    }
}

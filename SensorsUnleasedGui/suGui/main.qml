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
    width: 1024; //;Screen.width;
    height: 768; //Screen.height;
    color: suPalette.dark;
    title: qsTr("Sensors Unleased")

    SwipeView {
        id: swipeView
        anchors.fill: parent

        NodesPage{

        }
        onWidthChanged: console.log("SwipeView width: " + width + " height: " + height);
        onHeightChanged: console.log("SwipeView width: " + width + " height: " + height);
    }
}
